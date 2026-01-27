#ifndef MINIVIDEO_H
#define MINIVIDEO_H

#include <QVideoWidget>

class MiniVideo : public QVideoWidget
{
    Q_OBJECT
public:
    explicit MiniVideo(QWidget *parent = nullptr);
    void mouseMoveEvent(QMouseEvent* ev)override;
    void enterEvent(QEnterEvent* ev)override;
    void leaveEvent(QEvent* ev)override;
    QWidget* w_box;
signals:
};

#endif // MINIVIDEO_H
