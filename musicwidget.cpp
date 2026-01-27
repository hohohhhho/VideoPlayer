#include "musicwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QTimer>

MusicWidget::MusicWidget(QWidget *parent)
    : QWidget{parent}
{
    this->setStyleSheet("QToolButton{"
                        "border:none !important;"
                        "background-color:transparent;"
                        "}");

    this->btn_last=new QToolButton(this);
    this->btn_play=new QToolButton(this);
    this->btn_next=new QToolButton(this);

    btn_play->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn_play->setText("播放");

    const int psize=50;
    btn_last->setFixedSize(psize,psize);
    btn_play->setFixedSize(psize,psize);
    btn_next->setFixedSize(psize,psize);

    btn_last->setIconSize(QSize(psize,psize)*0.9);
    btn_play->setIconSize(QSize(psize,psize)*0.9);
    btn_next->setIconSize(QSize(psize,psize)*0.9);

    btn_last->setIcon(QIcon(":/res/last.png"));
    btn_play->setIcon(QIcon(":/res/start.png"));
    btn_next->setIcon(QIcon(":/res/next.png"));

    this->btn_last->setVisible(false);
    this->btn_play->setVisible(false);
    this->btn_next->setVisible(false);

    this->timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        static int index=0;
        QString qstr_pxp=":/res/music_playing/"+QString::number(index++)+".png";
        if(index>249){
            index=0;
        }
        this->pxp_current=QPixmap(qstr_pxp).scaled(this->size());
        update();
    });
}

void MusicWidget::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // QColor color1(100,100,210);//浅色
    QColor color2(80,80,170,180);//深色

    const int w_pad=width()/2*0.042;
    const int h_pad=height()/2*0.042;
    QRect rect=QRect(width()/4+w_pad , height()/4+h_pad , width()/2-w_pad*2 , height()/2-h_pad*2);
    // painter.setPen(color1);
    // painter.setBrush(color1);
    // painter.drawEllipse(rect);

    painter.setPen(color2);
    painter.setBrush(color2);
    int angle_start=90*16;
    int angle_span = -music_progress * 360 * 16;
    // qDebug()<<"span"<<angle_span/16;
    painter.drawPie(rect,angle_start,angle_span);

    if(!pxp_current.isNull()){
        painter.drawPixmap(0,0,pxp_current);
    }
}

void MusicWidget::resizeEvent(QResizeEvent *ev)
{
    const int psize=btn_last->width();
    btn_last->move(ev->size().width()/2-psize*2,ev->size().height()/2-psize/2);
    btn_play->move(ev->size().width()/2-psize/2,ev->size().height()/2-psize/2);
    btn_next->move(ev->size().width()/2+psize*1,ev->size().height()/2-psize/2);
}

void MusicWidget::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button()==Qt::LeftButton){
        changing_progress=true;
        // temp_progress_point=ev->pos();
        mouseTriggered(ev->pos());
    }
}

void MusicWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if(ev->buttons()&Qt::LeftButton){
        mouseTriggered(ev->pos());
    }
}

void MusicWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    // Q_UNUSED(ev);
    if(ev->button()==Qt::LeftButton){
        changing_progress=false;
        emit progessChanged(music_progress);
    }
}

void MusicWidget::enterEvent(QEnterEvent *ev)
{
    Q_UNUSED(ev);
    this->btn_last->show();
    this->btn_play->show();
    this->btn_next->show();
}

void MusicWidget::leaveEvent(QEvent *ev)
{
    Q_UNUSED(ev);
    this->btn_last->setVisible(false);
    this->btn_play->setVisible(false);
    this->btn_next->setVisible(false);
}

void MusicWidget::playGif(int frequent)
{
    this->timer->start(frequent);
}

void MusicWidget::stopGif()
{
    this->timer->stop();
}

void MusicWidget::setProgress(float progress)
{
    if(!changing_progress){
        this->music_progress=progress;
        // qDebug()<<"progress"<<progress;
        update();
    }
}

void MusicWidget::mouseTriggered(QPoint point)
{
    const QRect rect = QRect(width()/4, height()/4, width()/2, height()/2);
    QPainterPath path;
    path.addEllipse(rect);
    if(path.contains(point)){
        QPoint center=this->rect().center();
        double dx = point.x() - center.x();
        double dy = point.y() - center.y();

        double angle = qRadiansToDegrees(qAtan2(dy, dx));//下正上负
        qDebug()<<"angle"<<angle;
        angle+=90;

        if(angle<0){
            angle+=360;
        }

        this->music_progress=angle/360.0;

        update();
    }
}
