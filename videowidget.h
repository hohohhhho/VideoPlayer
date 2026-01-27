#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QVideoFrame>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent* ev)override;

    void setFrame(const QVideoFrame &frame);

signals:

private:
    QPixmap pxp_frame;
};

#endif // VIDEOWIDGET_H
