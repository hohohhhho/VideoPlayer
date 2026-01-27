#include "headbutton.h"

#include <QPainter>
#include <QResizeEvent>

HeadButton::HeadButton(QWidget *parent):QWidget(parent) {
    color_background=QColor(210,210,210);
    color_normal=QColor(100,100,100);
    color_clicked=QColor(210,210,210);
    color_hover=QColor(130,130,130);

    this->setMouseTracking(true);
}

void HeadButton::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);
    painter.fillRect(this->rect(),color_background);
    painter.fillRect(x_start,0,w_btn*4,height(),color_normal);
    if(this->index_hovering>=0 && index_hovering<4){
        painter.setBrush(color_hover);
        painter.setPen(color_hover);
        painter.drawRect(x_start+index_hovering*w_btn,0,w_btn,this->height());
    }
    if(this->index_clicking>=0 && index_clicking<4){
        painter.setBrush(color_clicked);
        painter.setPen(color_clicked);
        painter.drawRect(x_start+index_clicking*w_btn,0,w_btn,this->height());
    }
    if(this->x_start>0 && w_btn>0){
        const double pad=w_btn*0.1;
        QImage image_unfixed=fixed?QImage(":res/fixed.png"):QImage(":res/unfixed.png");
        QImage image_min(":res/min.png");
        QImage image_scaled(":res/scaled.png");
        QImage image_close(":res/close.png");
        if(this->theme_white){
            if(!image_unfixed.isNull()){
                for(int y=0;y<image_unfixed.height();y++)
                    for(int x=0;x<image_unfixed.width();x++){
                        QColor color=image_unfixed.pixelColor(x,y);
                        if(color.alpha()!=0){
                            image_unfixed.setPixelColor(x,y,QColor(210,210,210));
                        }
                    }
            }
            if(!image_min.isNull()){
                for(int y=0;y<image_min.height();y++)
                    for(int x=0;x<image_min.width();x++){
                        QColor color=image_min.pixelColor(x,y);
                        if(color.alpha()!=0){
                            image_min.setPixelColor(x,y,QColor(210,210,210));
                        }
                    }
            }
            if(!image_scaled.isNull()){
                for(int y=0;y<image_scaled.height();y++)
                    for(int x=0;x<image_scaled.width();x++){
                        QColor color=image_scaled.pixelColor(x,y);
                        if(color.alpha()!=0){
                            image_scaled.setPixelColor(x,y,QColor(210,210,210));
                        }
                    }
            }
            if(!image_close.isNull()){
                for(int y=0;y<image_close.height();y++)
                    for(int x=0;x<image_close.width();x++){
                        QColor color=image_close.pixelColor(x,y);
                        if(color.alpha()!=0){
                            image_close.setPixelColor(x,y,QColor(210,210,210));
                        }
                    }
            }
        }
        QPixmap pxp_unfixed=QPixmap::fromImage(image_unfixed);
        QPixmap pxp_min=QPixmap::fromImage(image_min);
        QPixmap pxp_scaled=QPixmap::fromImage(image_scaled);
        QPixmap pxp_close=QPixmap::fromImage(image_close);
        painter.drawPixmap(x_start+pad,pad,w_btn-pad*2,height()-pad*2,pxp_unfixed);
        painter.drawPixmap(x_start+pad+w_btn,pad,w_btn-pad*2,height()-pad*2,pxp_min);
        painter.drawPixmap(x_start+pad+w_btn*2,pad,w_btn-pad*2,height()-pad*2,pxp_scaled);
        painter.drawPixmap(x_start+pad+w_btn*3,pad,w_btn-pad*2,height()-pad*2,pxp_close);
    }

}

void HeadButton::resizeEvent(QResizeEvent *ev)
{
    this->w_btn=ev->size().height();
    this->x_start=ev->size().width()-this->w_btn*4;
    update();
}

void HeadButton::mousePressEvent(QMouseEvent *ev)
{
    if(x_start>0 && w_btn>0){
        if(ev->button()==Qt::LeftButton && ev->pos().x()>x_start){
            int dx=ev->pos().x()-x_start;
            this->index_clicking=dx/w_btn;
            update();
            // qDebug()<<"dx/w_btn"<<dx/w_btn;
        }
    }
    ev->ignore();
}

void HeadButton::mouseMoveEvent(QMouseEvent *ev)
{
    if(x_start>0 && w_btn>0){
        if(ev->pos().x()>x_start){
            int dx=ev->pos().x()-x_start;
            this->index_hovering=dx/w_btn;
            update();
            // qDebug()<<"dx/w_btn"<<dx/w_btn;
        }
    }
    ev->ignore();
}

void HeadButton::mouseReleaseEvent(QMouseEvent *ev)
{
    if(x_start>0 && w_btn>0){
        if(ev->button()==Qt::LeftButton && ev->pos().x()>x_start){
            int dx=ev->pos().x()-x_start;
            this->index_clicking=dx/w_btn;
            if(index_clicking==0){
                fixed=!fixed;
            }
            update();
            emit clicked(index_clicking);
            this->index_clicking=this->index_hovering=-1;
        }
    }
    ev->ignore();
}

void HeadButton::leaveEvent(QEvent *ev)
{
    Q_UNUSED(ev);
    this->index_clicking=this->index_hovering=-1;
    update();
}

void HeadButton::changeTheme(bool theme_white)
{
    this->theme_white=theme_white;
    if(theme_white){
        color_background=QColor(210,210,210);
        color_normal=QColor(100,100,100);
    }else{
        color_background=QColor(100,100,100);
        color_normal=QColor(210,210,210);
    }
    update();
}
