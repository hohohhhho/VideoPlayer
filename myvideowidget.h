#ifndef MYVIDEOWIDGET_H
#define MYVIDEOWIDGET_H

#include "widgetbox.h"
#include "progressbar.h"
#include "videobutton.h"

#include <QQueue>
#include <QSequentialAnimationGroup>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QVideoWidget>

class MyVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyVideoWidget(QWidget *parent = nullptr);
    ~MyVideoWidget();
    // bool eventFilter(QObject *obj, QEvent *event)override;
    void enterEvent(QEnterEvent* ev)override;
    void leaveEvent(QEvent* ev)override;
    void mouseMoveEvent(QMouseEvent* ev)override;
    void resizeEvent(QResizeEvent* ev)override;
    void paintEvent(QPaintEvent* ev)override;



    struct SrtFile{
        QString text;
        QTime start;
        QTime end;
    };
    QList<SrtFile*> list_srt;
    ProgressBar* bar;
    WidgetBox* w_box;
    VideoWidget* video_widget;
public slots:
    void updateCurrentTime(QTime time);
signals:


private:
    QVBoxLayout* main_layout;
    QLabel* label_srt;


    QSequentialAnimationGroup* group_show_box;
    QQueue<QVariantAnimation*> queue;
    void tryPlayShowBoxAnimation();

};

#endif // MYVIDEOWIDGET_H
