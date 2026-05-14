#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videobutton.h"
// #include "whisperthread.h"
#include "emotionanalysisworker.h"


#include <QAudioSource>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QPainter>
#include <QQueue>
#include <QThread>
#include <QVariantAnimation>

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void paintEvent(QPaintEvent* ev)override;
    void keyPressEvent(QKeyEvent* ev)override;
    void mouseDoubleClickEvent(QMouseEvent* ev)override;
    void dragEnterEvent(QDragEnterEvent* ev)override;
    void dropEvent(QDropEvent* ev)override;
    void resizeEvent(QResizeEvent* ev)override;
    void moveEvent(QMoveEvent* ev)override;
    void mousePressEvent(QMouseEvent* ev)override;
    void mouseMoveEvent(QMouseEvent* ev)override;
    void mouseReleaseEvent(QMouseEvent* ev)override;
    void closeEvent(QCloseEvent* ev)override;
    // bool eventFilter(QObject *obj, QEvent *ev)override;


    void setIndex(int new_index);
    void sendMsg(QString msg);
    void addFile(QString filename);
    void addSrt(QString filename);
    void fillScreen();
    void escScreen();
    void loadHistoryFromfile();
    void updateSource(QString filename);
    void loadNextVideo();
    void loadNextHistoryVideo();
    void deleteFilesFromPath(const QString& path);
    void switchVideoStack(QWidget* page);
    void loadUserInfo();
    void saveUserInfo();
    void changeBackgroundkPixmap(const QString &pxp);
    void changeFont(const QString& font);
    QString getAccessToken(const QString &url, const QString &apiKey, const QString &secretKey);
    void uploadToCOS(const QString &localPath, const QString &presignedUrl);
    QString generateAuthorization(const QString &secretId, const QString &secretKey, const QString &httpMethod, const QString &key, const QString &bucket, const QString &region);
    void uploadFileToCOS(const QString &filePath, const QString &bucket, const QString &region, const QString &secretId, const QString &secretKey);
private:
    Ui::MainWindow *ui;
    QMediaPlayer* player;
    QAudioSource* audio_source;
    QVideoSink* sink;
    QIODevice* audioIO;
    QNetworkAccessManager* manager;

    QMutex mutex_dlg_emotion;
    QDialog* dlg_emotion;
    QStringList list_media;
    QStringList list_history;
    QPixmap pxp_background;

    QMutex mutex_load_video;
    QQueue<VideoButton*> queue_load_video;//待加载的预览按钮队列
    QQueue<VideoButton*> queue_load_video_history;//待加载的历史预览按钮队列
    EmotionAnalysisWorker* worker_emo;
    QThread* thread_emo;
    QList<QRect> current_faces;
    QHash<QRect, QString> current_emotions;
    QVariantAnimation* animation_mask;
    int index_list_media = -1;//当前所播放的视频在list中的索引
    int play_mod = 1;//默认顺序播放
    bool full_screen = false;//是否去哪瓶
    bool loadingVideo = false;//是否正在加载视频
    bool user_setting_exchanged = false;//用户配置是否更改了
    QPoint current_pos;
    QSize current_size;
    QPoint start_point;
    QPoint start_pos;

    //用户信息：
    QString dir_save_history;//历史记录保存路径
    int max_history_num=30;//历史记录保存上限
    QMutex mutex_max_num_load_video_once;
    int max_num_load_video_once=5;//一次处理的视频数
    QMutex mutex_interval_load_video;
    int interval_load_video=500;//处理每个视频的时间间隔;
    float min_lumilance=0.1;//生成封面的最小亮度
    bool isWhite=true;//主题是否为浅色
    QString background_path=":/res/background.png";//背景图路径
    QString font_family="微软雅黑";
    int forward_sec=5;//一次快进的秒数
    int backward_sec=5;//一次快退的秒数
    //全局变量用户列表
};

#endif // MAINWINDOW_H
