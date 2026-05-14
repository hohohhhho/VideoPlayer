#include "myvideowidget.h"
#include "widgetbox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMutex>
#include <QPainter>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QVariantAnimation>

QMutex mutex_animation_show_box;

MyVideoWidget::MyVideoWidget(QWidget *parent)
    : QWidget{parent}
{
    main_layout=new QVBoxLayout(this);
    main_layout->setContentsMargins(0,0,0,0);
    main_layout->setSpacing(0);

    group_show_box=new QSequentialAnimationGroup(this);
    video_widget=new VideoWidget(this);
    bar=new ProgressBar(this);
    w_box=new WidgetBox(this);
    label_srt=new QLabel(video_widget);
    label_srt->setAlignment(Qt::AlignCenter);
    label_srt->setWordWrap(true);
    label_srt->setVisible(false);

    w_box->setVisible(false);

    main_layout->addWidget(video_widget);
    main_layout->addWidget(bar,9);
    main_layout->addWidget(w_box);



    connect(group_show_box,&QSequentialAnimationGroup::finished,this,[=](){
        QList<QAbstractAnimation*> list_delete;
        for(int i=0;i<group_show_box->animationCount();i++){
            list_delete.append(group_show_box->animationAt(i));
        }
        qDeleteAll(list_delete);
        group_show_box->clear();

        if(!queue.isEmpty()){
            group_show_box->addAnimation(queue.dequeue());
            group_show_box->start();
        }
    });

    this->setMouseTracking(true);
}

MyVideoWidget::~MyVideoWidget()
{
    qDeleteAll(list_srt);
    delete w_box;
}

void MyVideoWidget::enterEvent(QEnterEvent *ev)
{
    Q_UNUSED(ev);

    tryPlayShowBoxAnimation();

}

void MyVideoWidget::leaveEvent(QEvent *ev)
{
    Q_UNUSED(ev);
    QPoint global=this->mapToGlobal(QPoint(0,0));
    QRect rect(global.x(),global.y(),width(),height());
    //qDebug()<<"rect"<<rect<<"QCursor::pos()"<<QCursor::pos();
    if(rect.contains(QCursor::pos())){
        ev->accept();
        return;
        //qDebug()<<"ev->ignore();";
    }
    if(w_box->isVisible()){
        //qDebug()<<"leaveEvent";
        QVariantAnimation* animation_box_disappear=new QVariantAnimation(this);
        animation_box_disappear->setDuration(200);
        // animation_box_disappear->setStartValue(/*this->mapToGlobal(QPoint(0,0)).y()+*/this->height()-this->height()/12);
        // animation_box_disappear->setEndValue(/*this->mapToGlobal(QPoint(0,0)).y()+*/this->height());
        animation_box_disappear->setStartValue(w_box->btn_play->width());
        animation_box_disappear->setEndValue(1);
        connect(animation_box_disappear,&QVariantAnimation::valueChanged,this,[=](const QVariant& value){
            // this->w_box->move(0,value.toInt());
            main_layout->setStretchFactor(w_box,value.toInt());
        });
        connect(animation_box_disappear,&QVariantAnimation::finished,this,[=](){
            w_box->setVisible(false);
        });
        QMutexLocker locker(&mutex_animation_show_box);
        queue.append(animation_box_disappear);

        if(group_show_box->state()!=QVariantAnimation::Running){
            group_show_box->addAnimation(queue.dequeue());
            group_show_box->start();
        }
        locker.unlock();
    }
}

void MyVideoWidget::mouseMoveEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    // if(this->rect().contains(ev->globalPosition().toPoint())){

    // }
    // if(group_show_box->state()!=QSequentialAnimationGroup::Running && !w_box->isVisible()){
        tryPlayShowBoxAnimation();
    // }
}

void MyVideoWidget::resizeEvent(QResizeEvent *ev)
{
    //qDebug()<<"resize";
    main_layout->setStretchFactor(video_widget,ev->size().height());
    //w_box->move(this->mapToGlobal(QPoint(0,height()-height()/12)));

    this->w_box->resize(ev->size().width(),w_box->height());
    this->bar->resize(ev->size().width(),bar->height());
    this->w_box->setVisible(false);
    this->label_srt->resize(ev->size().width(),w_box->height());

    label_srt->move(0,video_widget->height()-label_srt->height()*3/2);
    // label_srt->adjustSize();
    //qDebug()<<"w_box->width()"<<w_box->width();
    //qDebug()<<"w_box"<<w_box->isVisible();
    //qDebug()<<"container->width()"<<container->width();
    //this->w_box->setFixedSize(ev->size().width(),this->height()/12);
}

void MyVideoWidget::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    // QPainter painter(this);
    // painter.fillRect(w_box->geometry(),Qt::red);
}

void MyVideoWidget::updateCurrentTime(QTime time)
{
    // qDebug()<<"update time"<<time<<"label"<<label_srt->text();
    // qDebug()<<"list size"<<list_srt.size();
    for(SrtFile*& file : list_srt){
        if(time>file->end && label_srt->text()==file->text){
            label_srt->setVisible(false);
        }
        if(time>=file->start && time<=file->end){
            label_srt->setText(file->text);
            label_srt->show();
            break;
        }
        // qDebug()<<"list_srt"<<list_srt;
    }
}


void MyVideoWidget::tryPlayShowBoxAnimation()
{
    if(group_show_box->state()==QSequentialAnimationGroup::Running || w_box->isVisible()){
        return;
    }
    //qDebug()<<"raise animation:btn width"<<w_box->btn_start->width();
    QVariantAnimation* animation_box_show=new QVariantAnimation(this);
    animation_box_show->setDuration(200);
    // animation_box_show->setStartValue(/*this->mapToGlobal(QPoint(0,0)).y()+*/this->height());
    // animation_box_show->setEndValue(/*this->mapToGlobal(QPoint(0,0)).y()+*/this->height()-this->height()/12);
    animation_box_show->setStartValue(1);
    animation_box_show->setEndValue(w_box->btn_play->width());
    //qDebug()<<"w_box->geometry()"<<w_box->geometry();
    //w_box->raise();
    w_box->show();
    //w_box->raise();
    connect(animation_box_show,&QVariantAnimation::valueChanged,this,[=](const QVariant& value){
        //this->w_box->move(/*this->mapToGlobal(QPoint(0,0)).x()*/0,value.toInt());
        main_layout->setStretchFactor(w_box,value.toInt());
        //qDebug()<<"value"<<value.toInt()<<"height"<<w_box->height();
        update();
    });
    QMutexLocker locker(&mutex_animation_show_box);
    queue.append(animation_box_show);

    if(group_show_box->state()!=QVariantAnimation::Running){
        group_show_box->addAnimation(queue.dequeue());
        group_show_box->start();
    }
    locker.unlock();
}
