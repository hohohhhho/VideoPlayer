#ifndef MUSICWIDGET_H
#define MUSICWIDGET_H

#include <QToolButton>
#include <QWidget>

class MusicWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MusicWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent* ev)override;
    void resizeEvent(QResizeEvent* ev)override;

    void mousePressEvent(QMouseEvent* ev)override;
    void mouseMoveEvent(QMouseEvent* ev)override;
    void mouseReleaseEvent(QMouseEvent* ev)override;
    void enterEvent(QEnterEvent* ev)override;
    void leaveEvent(QEvent* ev)override;

    void playGif(int frequent=50);
    void stopGif();

    QToolButton* btn_last;
    QToolButton* btn_play;
    QToolButton* btn_next;

public slots:
    void setProgress(float progress);
signals:
    void progessChanged(float progress);
private:
    QTimer* timer;
    QPixmap pxp_current;
    QPoint temp_progress_point;
    float music_progress=0;//1.0为100%
    bool changing_progress=false;

    void mouseTriggered(QPoint point);
};

#endif // MUSICWIDGET_H
