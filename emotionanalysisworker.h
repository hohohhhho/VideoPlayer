#ifndef EMOTIONANALYSISWORKER_H
#define EMOTIONANALYSISWORKER_H

#include <QHash>
#include <QObject>
#include <QSize>
#include <QVideoFrame>
#include "opencv2/opencv.hpp"

class EmotionAnalysisWorker : public QObject
{
    Q_OBJECT
public:
    explicit EmotionAnalysisWorker(QObject *parent = nullptr);


public slots:
    void processFrame(const QVideoFrame &frame, const QSize &widgetSize);
    // void processImage(const QImage &image, const QSize &widgetSize);
signals:
    void analysisCompleted(QList<QRect> faces, QHash<QRect, QString> emotions);

private:
    cv::Ptr<cv::FaceDetectorYN> faceDetector;
    cv::dnn::Net faceDetectNet;
    cv::dnn::Net emotionNet;
    cv::CascadeClassifier faceCascade;
    bool modelsLoaded = false;
    QSize lastWidgetSize;

    QList<QRect> list_face;  // 存储所有人脸位置矩形
    QHash<QRect, QString> hash_emo; // 键值对存储人脸位置对应的情绪

    void loadModels();
    QString predictEmotion(const cv::Mat &faceROI);
    QRect convertToWidgetCoordinates(const cv::Rect &faceRect, const QSize &frameSize, const QSize &widgetSize);
};

#endif // EMOTIONANALYSISWORKER_H
