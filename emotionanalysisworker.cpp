#include "emotionanalysisworker.h"
#include <QVideoFrame>
#include <QFile>
#include <QDebug>
#include <QScopeGuard>

extern const QString currentItemPath;

EmotionAnalysisWorker::EmotionAnalysisWorker(QObject *parent) : QObject(parent)
{
    loadModels();
}

void EmotionAnalysisWorker::loadModels()
{
    const QString yuNetPath = currentItemPath + "/models/face_detection/face_detection_yunet_2023mar.onnx";
    const QString emotionModelPath = currentItemPath + "/models/ssd.models/emotion-ferplus-8.onnx";

    if(!QFile::exists(yuNetPath) || !QFile::exists(emotionModelPath)) {
        qDebug() << "未找到模型，请检查路径";
        return;
    }

    try {
        // 参数依次为：模型路径、配置、输入尺寸（标准的为320*320）、置信度阈值(0.8)、NMS阈值(0.3)、保留前K个结果(5000)
        faceDetector = cv::FaceDetectorYN::create(
            yuNetPath.toStdString(),
            "",
            cv::Size(320, 320),
            0.8f,
            0.3f,
            5000
            );

        // 初始化 情绪Net
        emotionNet = cv::dnn::readNetFromONNX(emotionModelPath.toStdString());
        emotionNet.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        emotionNet.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        modelsLoaded = true;
        qDebug() << "ai模型加载成功.";
    } catch (const cv::Exception &e) {
        qDebug() << "捕捉到错误:" << e.what();
    }
}

void EmotionAnalysisWorker::processFrame(const QVideoFrame& frame, const QSize &widgetSize)
{
    if (!modelsLoaded || !frame.isValid()) return;

    QVideoFrame cloneFrame = frame;
    if (!cloneFrame.map(QVideoFrame::ReadOnly)) return;
    QImage image = cloneFrame.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat originalBgr;
    cv::Mat rgbFrame(image.height(), image.width(), CV_8UC3, (void*)image.bits(), image.bytesPerLine());
    cv::cvtColor(rgbFrame, originalBgr, cv::COLOR_RGB2BGR);
    cloneFrame.unmap();

    const int targetDim = 640;
    double scale = 1.0;
    cv::Mat detectionMat;

    if (originalBgr.cols > targetDim || originalBgr.rows > targetDim) {
        scale = static_cast<double>(targetDim) / std::max(originalBgr.cols, originalBgr.rows);
        cv::resize(originalBgr, detectionMat, cv::Size(), scale, scale, cv::INTER_LINEAR);
    } else {
        detectionMat = originalBgr.clone();
        scale = 1.0;
    }

    faceDetector->setInputSize(detectionMat.size());
    cv::Mat faces;
    faceDetector->detect(detectionMat, faces);

    QList<QRect> widgetFaces;
    QHash<QRect, QString> emotions;

    for (int i = 0; i < faces.rows; i++) {
        // 获取缩放图上的坐标
        float detX = faces.at<float>(i, 0);
        float detY = faces.at<float>(i, 1);
        float detW = faces.at<float>(i, 2);
        float detH = faces.at<float>(i, 3);
        float confidence = faces.at<float>(i, 14);
        if (confidence < 0.6) continue;
        // 映射回原图坐标 (关键步骤：除以 scale)
        int realX = static_cast<int>(detX / scale);
        int realY = static_cast<int>(detY / scale);
        int realW = static_cast<int>(detW / scale);
        int realH = static_cast<int>(detH / scale);
        cv::Rect faceRect(realX, realY, realW, realH);
        // 边界检查（防止缩放取整导致的微小越界）
        cv::Rect safeRect = faceRect & cv::Rect(0, 0, originalBgr.cols, originalBgr.rows);
        if (safeRect.width <= 10 || safeRect.height <= 10) continue;
        // 情绪识别 (在原图的高像素 ROI 上进行，保证准确度)
        cv::Mat faceROI = originalBgr(safeRect);
        QString emotion = predictEmotion(faceROI);
        // 转换到 UI 坐标 (传入的是相对于原图的坐标)
        QRect widgetRect = convertToWidgetCoordinates(
            faceRect,
            QSize(originalBgr.cols, originalBgr.rows),
            widgetSize
            );
        widgetFaces.append(widgetRect);
        emotions.insert(widgetRect, emotion);
    }

    emit analysisCompleted(widgetFaces, emotions);
}

QString EmotionAnalysisWorker::predictEmotion(const cv::Mat& faceROI)
{
    if (faceROI.empty()) return "Unknown";

    // 灰度化
    cv::Mat gray;
    if (faceROI.channels() == 3) {
        cv::cvtColor(faceROI, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceROI;
    }

    // 对比度增强
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    cv::Mat equalized;
    clahe->apply(gray, equalized);
    // 缩放尺寸
    cv::Mat resized;
    cv::resize(equalized, resized, cv::Size(64, 64), 0, 0, cv::INTER_AREA);
    // 归一化
    cv::Mat blob = cv::dnn::blobFromImage(resized, 1.0, cv::Size(64, 64), cv::Scalar(0), false, false);
    // 推理
    emotionNet.setInput(blob);
    cv::Mat prob = emotionNet.forward();
    // 解析结果
    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(prob, nullptr, &confidence, nullptr, &classIdPoint);
    int classId = classIdPoint.x;

    static const QStringList emotionLabels = {// 标签映射
        "Neutral", "Happiness", "Surprise", "Sadness",
        "Anger", "Disgust", "Fear", "Contempt"
    };

    if (classId >= 0 && classId < emotionLabels.size()) {
        return emotionLabels[classId];
    }
    return "Neutral";
}

QRect EmotionAnalysisWorker::convertToWidgetCoordinates(const cv::Rect &faceRect,const QSize &frameSize,const QSize &widgetSize)
{
    // 计算实际显示区域
    double frameAspect = (double)frameSize.width() / frameSize.height();
    double widgetAspect = (double)widgetSize.width() / widgetSize.height();

    int actualWidth, actualHeight;
    if(frameAspect > widgetAspect) {
        actualWidth = widgetSize.width();
        actualHeight = widgetSize.width() / frameAspect;
    } else {
        actualHeight = widgetSize.height();
        actualWidth = widgetSize.height() * frameAspect;
    }

    // 计算偏移量（居中显示）
    int xOffset = (widgetSize.width() - actualWidth) / 2;
    int yOffset = (widgetSize.height() - actualHeight) / 2;

    // 计算缩放比例
    double scaleX = (double)actualWidth / frameSize.width();
    double scaleY = (double)actualHeight / frameSize.height();

    return QRect(
        faceRect.x * scaleX + xOffset,
        faceRect.y * scaleY + yOffset,
        faceRect.width * scaleX,
        faceRect.height * scaleY
        );
}
