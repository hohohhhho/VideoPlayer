#include "gifwidget.h"

#include <QPainter>
#include <QTimer>

GifWidget::GifWidget(QWidget *parent)
    : QWidget{parent}
{
    this->timer=new QTimer(this);
}

void GifWidget::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);

    if(playing){
        //qDebug()<<"current_index"<<current_index;
        painter.drawPixmap(-30,0,this->width(),this->height(),this->pxp_current);
    }
}

void GifWidget::setSource(QString sourse, int end_index,int frequent, int start_index)
{
    this->end_index=end_index;
    this->start_index=start_index;
    this->current_index=start_index;
    this->frequent=frequent;

    connect(timer,&QTimer::timeout,this,[=]{
        QString qstr_sourse=QString(":/res/%1/%2.png").arg(sourse).arg(current_index);
        if(direction){
            current_index++;
        }else{
            current_index--;
        }

        if(current_index>end_index){
            direction=false;
            current_index--;
        }else if(current_index<start_index){
            direction=true;
            current_index++;
        }
        this->pxp_current=QPixmap(qstr_sourse);
        update();
    });

}

void GifWidget::start()
{
    this->playing=true;
    timer->start(frequent);
    update();
}

void GifWidget::stop()
{
    this->playing=false;
    timer->stop();
}
