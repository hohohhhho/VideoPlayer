#ifndef GIFWIDGET_H
#define GIFWIDGET_H

#include <QWidget>

class GifWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GifWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent* ev)override;

    void setSource(QString sourse,int end_index,int frequent,int start_index=0);
    void start();
    void stop();

    bool direction=true;//正向播放
signals:

private:
    QPixmap pxp_current;
    int start_index,end_index;
    int current_index;
    int frequent;
    bool playing=false;
    QTimer* timer;
};

#endif // GIFWIDGET_H
