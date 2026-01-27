#include "videobutton.h"

#include <QApplication>
#include <QEvent>
#include <QMutex>
#include <QPainter>
#include <QThread>
#include <QVBoxLayout>
#include <QVideoFrame>
#include <QScreen>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
// QMutex mutex_first_time;

extern QMutex mutex_user_list;
extern QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists;

VideoButton::VideoButton(QWidget *parent)
    : QWidget{parent}
{
    // mutex_init.lock();
    // this->setVisible(false);

    this->stack=new QStackedWidget(this);
    this->label_pxp=new QLabel(this);
    this->label_tip=nullptr;
    this->m_sink=new QVideoSink(this);
    this->player=new QMediaPlayer;
    //this->video_widget=new QVideoWidget(this);
    this->temp_video_widget=new VideoWidget(this);
    this->gif_widget=new GifWidget(this);

    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(stack);
    this->layout()->setAlignment(Qt::AlignCenter);

    stack->addWidget(label_pxp);
    stack->addWidget(temp_video_widget);
    stack->addWidget(gif_widget);
    stack->setCurrentWidget(label_pxp);

    label_pxp->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    label_pxp->setScaledContents(true);
    //label_pxp->setAlignment(Qt::AlignCenter);
    this->temp_video_widget->setVisible(false);
    this->label_pxp->setPixmap(QPixmap(":/res/loading.png"));
    this->gif_widget->setSource("playing",61,50,30);


    player->setVideoSink(m_sink);
    player->setPlaybackRate(3);
    // QTimer* checker=new QTimer(this);
    // connect(checker,&QTimer::timeout,this,[=](){
    //     mutex_loaded.lock();
    //     if(!player->isPlaying() && !loaded){
    //         player->play();
    //     }
    //     mutex_loaded.unlock();
    // });
    // checker->start(500);


    connect(this->m_sink,&QVideoSink::videoFrameChanged,this,[=](const QVideoFrame& frame){
        // qDebug()<<"loaded"<<loaded;
        // QMutexLocker lock(&mutex_loaded);
        mutex_loaded.lock();
        if(!loaded)
        {
            // thread->quit();
            // thread->wait();
            player->pause();
            qreal ratio=QGuiApplication::primaryScreen()->devicePixelRatio();
            QImage image=frame.toImage().scaled(label_pxp->size()*ratio,Qt::IgnoreAspectRatio,Qt::FastTransformation);
            image.setDevicePixelRatio(ratio);



            mutex_min_lumilance.lock();
            double min=min_lumilance;
            mutex_min_lumilance.unlock();

            double lumilance=1;
            if(min!=0){
                lumilance=getAverageLumilance(image);
            }
            // qDebug()<<"lumilance"<<lumilance;
            if(lumilance>min){//亮度阈值达标则不再取封面，否则继续播放视频找到亮度达标的帧
                player->setPlaybackRate(1);
                this->label_pxp->setPixmap(QPixmap::fromImage(image));
                update();
                loaded=true;
                // checker->stop();
                // checker->deleteLater();
                qDebug()<<name<<"获取到封面:"<<label_pxp;
            }else{
                player->play();
                qDebug()<<name<<"继续获取封面";
            }
            // QGuiApplication::processEvents();
            //qDebug()<<"label"<<label_pxp;
        }else{
            temp_video_widget->setFrame(frame);
        }
        mutex_loaded.unlock();
    });

    // temp_video_widget->installEventFilter(this);
    // stack->installEventFilter(this);
    // label_pxp->installEventFilter(this);
    // gif_widget->installEventFilter(this);

    temp_video_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
    stack->setAttribute(Qt::WA_TransparentForMouseEvents);
    label_pxp->setAttribute(Qt::WA_TransparentForMouseEvents);
    gif_widget->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->setMouseTracking(true);

    label_pxp->show();
    // QTimer::singleShot(10,this,[=](){
    //
    //     mutex_init.unlock();
    //     this->init=true;
    //     this->setVisible(true);
    // });

}

VideoButton::~VideoButton()
{
    player->stop();
    // if(thread){
    //     thread->quit();
    //     thread->wait();
    //     thread->deleteLater();
    // }
}

void VideoButton::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);
    // painter.fillRect(0,0,this->width(),this->height(),Qt::red);
    // painter.fillRect(label_pxp->geometry(),Qt::blue);
    // qDebug()<<"label_pxp->geometry()"<<mapToGlobal(label_pxp->pos());
    // qDebug()<<"this->geometry()"<<mapToGlobal(this->pos());
    // qDebug()<<"stack->geometry()"<<mapToGlobal(stack->pos());
    // qDebug()<<"label_pxp->pixmap().rect()"<<mapToGlobal(QPoint(label_pxp->pixmap().rect().x(),label_pxp->pixmap().rect().y()));

    //qDebug()<<"pxp_cover"<<pxp_cover;
    // if(!pxp_cover.isNull()){
    //     painter.drawPixmap(0,0,this->width(),this->height(),this->pxp_cover);
    // }
}

void VideoButton::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    if(ev->button()==Qt::LeftButton){
        // qDebug()<<"pressed";
        // if(thread->isRunning()){
        //     stack->setCurrentWidget(video_widget);
        //     thread->quit();
        //     thread->wait();
        // }
        player->stop();
        if(player->isPlaying()){
            qDebug()<<"press isplaying";
        }
        emit clicked();
    }
}

void VideoButton::mouseMoveEvent(QMouseEvent *ev)
{
    // qDebug()<<"move";
    // if(!mutex_init.tryLock()){
    //     return;
    // }else{
    //     mutex_init.unlock();
    // }
    if(enter && label_tip!=nullptr){
        label_tip->move(ev->globalPosition().toPoint()+QPoint(1,1));//防止鼠标位于label上反复触发leaveEvent
    }
}

void VideoButton::enterEvent(QEnterEvent *ev)
{
    Q_UNUSED(ev);
    // if(!mutex_init.tryLock()){
    //     return;
    // }else{
    //     mutex_init.unlock();
    // }
    enter=true;
    if(isMusic){
        label_tip=new QLabel;
        label_tip->setWindowFlag(Qt::FramelessWindowHint);
        label_tip->setText(this->name);
        label_tip->setAlignment(Qt::AlignCenter);
        label_tip->move(ev->globalPosition().toPoint()+QPoint(1,1));
        label_tip->show();
    }else if(stack->currentWidget()!=gif_widget){
        stack->setCurrentWidget(temp_video_widget);
        // thread->start();
        player->play();
    }
}

void VideoButton::leaveEvent(QEvent *ev)
{
    Q_UNUSED(ev);
    // if(!mutex_init.tryLock()){
    //     return;
    // }else{
    //     mutex_init.unlock();
    // }
    // qDebug()<<"leave";
    enter=false;
    if(stack->currentWidget()!=gif_widget){
        stack->setCurrentWidget(label_pxp);
        player->pause();
    }
    if(label_tip){
        delete label_tip;
        label_tip=nullptr;
    }

    // thread->quit();
    // thread->wait();

}

bool VideoButton::eventFilter(QObject *obj, QEvent *event){
    // if(!mutex_init.tryLock()){
    //     event->accept();
    //     return true;
    // }else{
    //     mutex_init.unlock();
    // }
    if (event->type()==QEvent::MouseButtonPress ||event->type()==QEvent::Enter ||event->type()==QEvent::Leave) {
        event->ignore();
        return false;
    }else if(event->type()==QEvent::MouseMove){
        qDebug()<<"mouseMove";
        // QMouseEvent* ev=dynamic_cast<QMouseEvent*>(event);
        // if(ev){
        //     qDebug()<<"filter:move event";
        //     VideoButton::mouseMoveEvent(ev);
        // }
        return true;
    }else {
        return QObject::eventFilter(obj, event);
    }
}

void VideoButton::contextMenuEvent(QContextMenuEvent *ev)
{
    mutex_user_list.lock();
    QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists_copy=user_lists;
    mutex_user_list.unlock();

    QMenu* menu=new QMenu(this);

    QAction* act_name=new QAction(name,menu);
    QAction* act_reget=new QAction("刷新",menu);
    QMenu* menu_add=new QMenu("添加至",menu);
    QAction* act_delete=new QAction("删除",menu);
    QMenu* menu_move=new QMenu("移动",menu);
    QAction* act_up=new QAction("上移",menu);
    QAction* act_down=new QAction("下移",menu);

    menu->addAction(act_name);
    menu->addSeparator();
    menu->addAction(act_reget);
    menu->addMenu(menu_add);
    menu->addAction(act_delete);
    menu->addMenu(menu_move);
    menu_move->addAction(act_up);
    menu_move->addAction(act_down);

    for(QPair<QString,QList< QPair<QString,QString>>>& pair:user_lists_copy){
        QAction* act_add_to_list=new QAction(pair.first,menu_add);
        menu_add->addAction(act_add_to_list);
        connect(act_add_to_list,&QAction::triggered,this,[=]()mutable{
            QString name=this->getSource().split(".").first().split("/").last();
            mutex_user_list.lock();
            for(QPair<QString,QList< QPair<QString,QString>>>& pair_2:user_lists){
                if(pair_2.first==pair.first){
                    pair_2.second.append(qMakePair(name,getSource()));
                }
            }
            mutex_user_list.unlock();
        });
    }

    connect(act_reget,&QAction::triggered,this,[=](){
        if(!isMusic){
            mutex_loaded.lock();
            this->loaded=false;
            mutex_loaded.unlock();
            this->label_pxp->setPixmap(QPixmap(""));
            this->playSource();
        }
    });
    connect(act_delete,&QAction::triggered,this,[=](){
        if(QMessageBox::StandardButton(QMessageBox::Yes)==
                        QMessageBox::information(this,"提示","确定要删除这条记录吗?"
                                    ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))){
            emit willDelete();
        }
    });
    connect(act_up,&QAction::triggered,this,[=](){
        emit movePosition(true);
    });
    connect(act_down,&QAction::triggered,this,[=](){
        emit movePosition(false);
    });

    menu->exec(ev->globalPos());
    menu->deleteLater();
}

void VideoButton::stopGif()
{
    this->stack->setCurrentWidget(label_pxp);
    this->gif_widget->stop();
}

void VideoButton::playGif()
{
    this->stack->setCurrentWidget(gif_widget);
    this->gif_widget->start();
}

void VideoButton::loadSource(QString Source)
{
    this->name=Source.split("/").last().split(".").first();
    this->player->setSource(QUrl::fromLocalFile(Source));
    mutex_loaded.lock();
    this->loaded=false;
    mutex_loaded.unlock();
    //qDebug()<<"button url:"<<QUrl::fromLocalFile(Source);
    // this->thread=new QThread(this);
    // player->moveToThread(thread);

    // connect(thread,&QThread::started,player,&QMediaPlayer::play);
    // connect(player,&QMediaPlayer::playbackStateChanged,this,[=](QMediaPlayer::PlaybackState state){
    //     if(state==QMediaPlayer::PausedState){
    //         thread->quit();
    //         thread->wait();
    //     }
    // });
    // connect(thread,&QThread::finished,player,&QMediaPlayer::pause);
    // connect(thread,&QThread::destroyed,player,&QMediaPlayer::deleteLater);

    //thread->start();

    //qDebug()<<player->videoSink();
}

void VideoButton::playSource()
{
    player->play();
}

void VideoButton::setMusic(bool isMusic)
{
    this->isMusic=isMusic;
    if(isMusic){
        this->label_pxp->setPixmap(QPixmap(":/res/music.png"));
        QMutexLocker locker(&mutex_loaded);
        this->loaded=true;
    }

}
bool VideoButton::isLoaded(){
    QMutexLocker lock(&mutex_loaded);
    return loaded;
}

void VideoButton::setMinLumilance(double min_lumilance)
{
    this->mutex_min_lumilance.lock();
    this->min_lumilance=min_lumilance;
    this->mutex_min_lumilance.unlock();
}

double VideoButton::getAverageLumilance(QImage image)
{
    double total=0.0;
    QColor temp_color;

    for(int y=0;y<image.height();y++)
        for(int x=0;x<image.width();x++){
            temp_color=image.pixelColor(x,y);
            //使用ITU-R BT.790标准亮度公式
            total+=0.299*temp_color.redF()+0.587*temp_color.greenF()+0.114*temp_color.blueF();
        }

    return total/(image.width()*image.height());
}
