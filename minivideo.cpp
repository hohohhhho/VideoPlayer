#include "minivideo.h"


#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>

MiniVideo::MiniVideo(QWidget *parent)
    : QVideoWidget{parent}
{
    this->setMouseTracking(true);
}

void MiniVideo::mouseMoveEvent(QMouseEvent *ev)
{
    ev->ignore();
}

void MiniVideo::enterEvent(QEnterEvent *ev)
{
    ev->ignore();
}

void MiniVideo::leaveEvent(QEvent *ev)
{
    ev->ignore();
}
