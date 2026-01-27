#include "progressbar.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget{parent}
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    this->setFixedHeight(max_pen_size);
}

void ProgressBar::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);
    painter.setPen(QPen(color,current_pen_size));


    if(progress!=0){
        //qDebug()<<"width()*progress"<<width()*progress;
        painter.drawLine(0,height()/2,width()*progress,height()/2);
    }
}

void ProgressBar::enterEvent(QEnterEvent *ev)
{
    Q_UNUSED(ev);
    this->current_pen_size=max_pen_size;
    update();
}

void ProgressBar::leaveEvent(QEvent *ev)
{
    Q_UNUSED(ev);
    this->current_pen_size=min_pen_size;
    update();
}

void ProgressBar::mousePressEvent(QMouseEvent *ev)
{
    this->progress=static_cast<double>(ev->pos().x())/width();
    emit changedProgress(progress);
    // qDebug()<<"width"<<width();
    // qDebug()<<"ev"<<ev->pos();
    update();
}

void ProgressBar::mouseMoveEvent(QMouseEvent *ev)
{
    this->progress=static_cast<double>(ev->pos().x())/width();
    emit changedProgress(progress);
    update();
}

void ProgressBar::setProgress(double progress)
{
    this->progress=progress;
    repaint();
}
