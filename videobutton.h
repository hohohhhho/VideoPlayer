#ifndef VIDEOBUTTON_H
#define VIDEOBUTTON_H

#include "videowidget.h"
#include "gifwidget.h"

#include <QWidget>
#include <QVideoSink>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QMutex>
#include <QTime>

class VideoButton : public QWidget
{
    Q_OBJECT
public:
    explicit VideoButton(QWidget *parent = nullptr);
    ~VideoButton();
    void paintEvent(QPaintEvent* ev)override;
    void mousePressEvent(QMouseEvent* ev)override;
    void mouseMoveEvent(QMouseEvent* ev)override;
    void enterEvent(QEnterEvent* ev)override;
    void leaveEvent(QEvent* ev)override;
    bool eventFilter(QObject *obj, QEvent *event)override;
    void contextMenuEvent(QContextMenuEvent* ev)override;

    void stopGif();
    void playGif();
    QString getSource(){return player->source().toLocalFile();}
    void loadSource(QString Source);
    void playSource();
    void setMusic(bool isMusic=true);
    bool isMusicButton(){return isMusic;};
    bool isLoaded();
    void setMinLumilance(double min_lumilance);
    QString name;
    QDateTime stamp;
signals:
    void clicked();
    void willDelete();
    void movePosition(bool up);
private:
    QStackedWidget* stack;
    QVideoSink* m_sink;
    QMediaPlayer* player;
    VideoWidget* temp_video_widget;
    GifWidget* gif_widget;

    //QVideoWidget* video_widget;
    QLabel* label_pxp;
    QLabel* label_tip;
    QThread* thread;

    QMutex mutex_loaded;
    bool loaded=false;
    QMutex mutex_min_lumilance;
    float min_lumilance=0.1;
    bool isMusic=false;
    bool enter=false;//判断鼠标进入
    bool init=false;
    QMutex mutex_init;

    //QPixmap pxp_cover;
    double getAverageLumilance(QImage image);

};

#endif // VIDEOBUTTON_H
