#include "emotionanalysisworker.h"
#include <QVideoFrame>
#include <QFile>
#include <QDebug>
#include <QScopeGuard>

EmotionAnalysisWorker::EmotionAnalysisWorker(QObject *parent) : QObject(parent)
{
    loadModels();
}

void EmotionAnalysisWorker::loadModels()
{
    // const QString haarPath = ":/models/haarcascade/haarcascade_frontalface_default.xml";
    // const QString emotionModelPath = ":/models/ssd.models/emotion-ferplus-8.onnx";
    const QString haarPath = "D:/Qt2.0/study1/project2/item26_video_player/models/haarcascade/haarcascade_frontalface_default.xml";
    // const QString yuNetPath = "D:/Qt2.0/study1/project2/item26_video_player/models/face_detection/face_detection_yunet_2023mar_int8bq.onnx";
    const QString emotionModelPath = "D:/Qt2.0/study1/project2/item26_video_player/models/ssd.models/emotion-ferplus-8.onnx";

    if(!QFile::exists(haarPath)) {
        qDebug() << "Haar cascade file not found:" << haarPath;
        return;
    }
    if(!QFile::exists(emotionModelPath)) {
        qDebug() << "Emotion model file not found:" << emotionModelPath;
        return;
    }
    // faceDetector = cv::FaceDetectorYN::create(
    //     yuNetPath.toStdString(),
    //     "",
    //     cv::Size(640, 480),  // 增大输入尺寸以提高小脸检测能力
    //     0.85f,  // 提高置信度阈值
    //     0.4f,   // 放宽NMS阈值
    //     5000
    //     );
    // try {
    //     emotionNet = cv::dnn::readNetFromONNX(emotionModelPath.toStdString());
    //     // 启用GPU加速
    //     emotionNet.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    //     emotionNet.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    //     // 验证模型是否加载成功
    //     if (emotionNet.empty()) {
    //         qDebug() << "Failed to load emotion model";
    //         return;
    //     }
    //     modelsLoaded = true;
    // } catch (const cv::Exception &e) {
    //     qDebug() << "OpenCV Exception:" << e.what();
    // }
    // modelsLoaded = !emotionNet.empty() && !faceDetector.empty();

    if(faceCascade.load(haarPath.toStdString())) {
        try {
            emotionNet = cv::dnn::readNetFromONNX(emotionModelPath.toStdString());
            // 启用GPU加速
            emotionNet.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            emotionNet.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

            // 验证模型是否加载成功
            if (emotionNet.empty()) {
                qDebug() << "Failed to load emotion model";
                return;
            }
            modelsLoaded = true;
        } catch (const cv::Exception &e) {
            qDebug() << "OpenCV Exception:" << e.what();
        }
    } else {
        qDebug() << "Failed to load Haar cascade";
    }
}

void EmotionAnalysisWorker::processFrame(const QVideoFrame frame, const QSize &widgetSize)
{
    if (!modelsLoaded || !frame.isValid()) return;

    QVideoFrame cloneFrame = frame;  // 显式克隆
    if (!cloneFrame.map(QVideoFrame::ReadOnly)) return;

    // 使用智能指针管理图像数据
    auto image = std::make_shared<QImage>(
        cloneFrame.toImage().convertToFormat(QImage::Format_RGB888)
        );

    cloneFrame.unmap();  // 立即解除映射

    // 异步处理图像
    cv::Mat cvFrame(image->height(), image->width(), CV_8UC3, image->bits());

    try {
        QImage image = cloneFrame.toImage().convertToFormat(QImage::Format_RGB888);
        cv::Mat cvFrame(
            image.height(),
            image.width(),
            CV_8UC3,
            image.bits(),
            image.bytesPerLine()
        );

        cv::cvtColor(cvFrame, cvFrame, cv::COLOR_RGB2BGR);

        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(cvFrame, faces, 1.1, 3, 0, cv::Size(30, 30));
        // faceCascade.detectMultiScale(
        //     cvFrame,
        //     faces,
        //     1.05,  // 缩小缩放步长（提高检测密度）
        //     5,     // 增加最小相邻矩形数（降低误检）
        //     cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH,
        //     cv::Size(100, 100),  // 最小人脸尺寸（根据应用场景调整）
        //     cv::Size(400, 400)   // 最大人脸尺寸
        //     );

        QList<QRect> widgetFaces;
        QHash<QRect, QString> emotions;

        for(const auto &faceRect : faces) {
            QRect widgetRect = convertToWidgetCoordinates(
                faceRect,
                QSize(cvFrame.cols, cvFrame.rows),
                widgetSize
            );

            cv::Mat faceROI = cvFrame(faceRect);
            QString emotion = predictEmotion(faceROI);

            widgetFaces.append(widgetRect);
            emotions.insert(widgetRect, emotion);
        }

        emit analysisCompleted(widgetFaces, emotions);
    }
    // try {
    //     // 设置输入尺寸
    //     faceDetector->setInputSize(cv::Size(cvFrame.cols, cvFrame.rows));

    //     // 执行检测
    //     cv::Mat faces;
    //     faceDetector->detect(cvFrame, faces);

    //     QList<QRect> widgetFaces;
    //     QHash<QRect, QString> emotions;

    //     // 解析检测结果
    //     for(int i = 0; i < faces.rows; ++i) {
    //         // 获取人脸矩形（x,y,w,h）
    //         // cv::Rect faceRect(
    //         //     faces.at<float>(i, 0),  // x
    //         //     faces.at<float>(i, 1),  // y
    //         //     faces.at<float>(i, 2),  // w
    //         //     faces.at<float>(i, 3)   // h
    //         //     );
    //         float x1 = faces.at<float>(i, 0);
    //         float y1 = faces.at<float>(i, 1);
    //         float x2 = faces.at<float>(i, 2);
    //         float y2 = faces.at<float>(i, 3);
    //         float conf = faces.at<float>(i, 4);

    //         // 跳过低置信度检测（提高阈值过滤无效结果）
    //         if (conf < 0.7) {  // 从0.6提高到0.7
    //             continue;
    //         }

    //         // 转换为整数坐标并计算实际宽高
    //         int ix1 = static_cast<int>(x1);
    //         int iy1 = static_cast<int>(y1);
    //         int ix2 = static_cast<int>(x2);  // 新增变量
    //         int iy2 = static_cast<int>(y2);  // 新增变量
    //         int width = ix2 - ix1;  // 正确计算宽度
    //         int height = iy2 - iy1; // 正确计算高度

    //         // 创建Rect并约束在图像范围内
    //         cv::Rect faceRect(ix1, iy1, width, height);
    //         faceRect = faceRect & cv::Rect(0, 0, cvFrame.cols, cvFrame.rows);

    //         // 验证区域有效性（调整最小尺寸阈值）
    //         if (faceRect.width <= 20 || faceRect.height <= 20) { // 从10调整到20像素
    //             qDebug() << "过滤小尺寸人脸区域：" << faceRect.size().width<<","<<faceRect.size().height;
    //             continue;
    //         }

    //         // 坐标转换
    //         QRect widgetRect = convertToWidgetCoordinates(
    //             faceRect,
    //             QSize(cvFrame.cols, cvFrame.rows),
    //             widgetSize
    //             );

    //         // 情绪分析
    //         cv::Mat faceROI = cvFrame(faceRect);
    //         QString emotion = predictEmotion(faceROI);

    //         widgetFaces.append(widgetRect);
    //         emotions.insert(widgetRect, emotion);
    //     }

    //     emit analysisCompleted(widgetFaces, emotions);
    // }
    catch (const cv::Exception& e) {
        qWarning() << "OpenCV Exception:" << e.what();
    }
    catch (const std::exception& e) {
        qWarning() << "STD Exception:" << e.what();
    }
}

QString EmotionAnalysisWorker::predictEmotion(const cv::Mat &faceROI)
{
    cv::Mat grayFace;
    cv::cvtColor(faceROI, grayFace, cv::COLOR_BGR2GRAY);

    // 调整尺寸
    cv::Mat resized;
    cv::resize(grayFace, resized, cv::Size(64, 64));

    // 数值转换：先除255再减0.5
    cv::Mat floatMat;
    resized.convertTo(floatMat, CV_32F);
    floatMat = floatMat / 255.0f;
    floatMat = floatMat - 0.5f;

    // 生成blob
    cv::Mat blob = cv::dnn::blobFromImage(
        floatMat,
        1.0,
        cv::Size(64, 64),
        cv::Scalar(),
        false,
        false,
        CV_32F
        );

    // 验证blob形状
    Q_ASSERT(blob.size[1] == 1 && blob.size[2] == 64 && blob.size[3] == 64);
    // 正确应输出：1 1 64 64

    // Step5: 执行推理
    emotionNet.setInput(blob);
    cv::Mat prob = emotionNet.forward();

    // 调试输出
    qDebug() << "Output probabilities:";
    for (int i = 0; i < (int)prob.total(); ++i) {
        qDebug() << "Class" << i << ":" << prob.at<float>(i);
    }

    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(prob, nullptr, &confidence, nullptr, &classIdPoint);

    const static QStringList emotions = {
        "Neutral", "Happy", "Surprise", "Sad",
        "Anger", "Disgust", "Fear", "Contempt"
    };

    int classId = classIdPoint.x;
    if (classId >= 0 && classId < emotions.size()) {
        qDebug() << "Predicted emotion:" << emotions[classId] << "with confidence:" << confidence;
        return emotions[classId];
    }
    return "Unknown";
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
