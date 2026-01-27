#include "widget.h"
#include "ui_widget.h"
#include "videobutton.h"
#include <QAudioOutput>
#include <QButtonGroup>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMediaPlayer>
#include <QMutex>
#include <QRadioButton>
#include <QRandomGenerator>
#include <QSlider>
#include <QStandardItemModel>
#include <QTimer>
#include <QVariantAnimation>
#include <QMimeData>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setWindowTitle("视频播放器");

    this->player=new QMediaPlayer(this);
    QAudioOutput* audio=new QAudioOutput(this);

    player->setAudioOutput(audio);
    player->setVideoOutput(ui->widget_video->video_widget);

    // QStandardItemModel* model=new QStandardItemModel(this);
    // ui->listView->setModel(model);
    {
        QButtonGroup* group=new QButtonGroup(this);
        QRadioButton* speed1=new QRadioButton("3.0",this);
        QRadioButton* speed2=new QRadioButton("2.0",this);
        QRadioButton* speed3=new QRadioButton("1.5",this);
        QRadioButton* speed4=new QRadioButton("1.25",this);
        QRadioButton* speed5=new QRadioButton("1.0",this);
        QRadioButton* speed6=new QRadioButton("0.5",this);
        speed5->setChecked(true);
        group->addButton(speed1,1);
        group->addButton(speed2,2);
        group->addButton(speed3,3);
        group->addButton(speed4,4);
        group->addButton(speed5,5);
        group->addButton(speed6,6);
        ui->widget_video->w_box->combo_speed->addWidget(speed1);
        ui->widget_video->w_box->combo_speed->addWidget(speed2);
        ui->widget_video->w_box->combo_speed->addWidget(speed3);
        ui->widget_video->w_box->combo_speed->addWidget(speed4);
        ui->widget_video->w_box->combo_speed->addWidget(speed5);
        ui->widget_video->w_box->combo_speed->addWidget(speed6);
        connect(speed1,&QPushButton::clicked,this,[=](){
            player->setPlaybackRate(speed1->text().toDouble());
            ui->widget_video->w_box->combo_speed->btn_main->setText(speed1->text());
        });
        connect(speed2,&QPushButton::clicked,this,[=](){
            player->setPlaybackRate(speed2->text().toDouble());
            ui->widget_video->w_box->combo_speed->btn_main->setText(speed2->text());
        });
        connect(speed3,&QPushButton::clicked,this,[=](){
            player->setPlaybackRate(speed3->text().toDouble());
            ui->widget_video->w_box->combo_speed->btn_main->setText(speed3->text());
        });
        connect(speed4,&QPushButton::clicked,this,[=](){
            player->setPlaybackRate(speed4->text().toDouble());
            ui->widget_video->w_box->combo_speed->btn_main->setText(speed4->text());
        });
        connect(speed5,&QPushButton::clicked,this,[=](){
            player->setPlaybackRate(speed5->text().toDouble());
            ui->widget_video->w_box->combo_speed->btn_main->setText(speed5->text());
        });
        connect(speed6,&QPushButton::clicked,this,[=](){
            player->setPlaybackRate(speed6->text().toDouble());
            ui->widget_video->w_box->combo_speed->btn_main->setText(speed6->text());
        });
    }
    // QPushButton* mainBtn_play_mod=new QPushButton("播放模式",this);
    // mainBtn_play_mod->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    // ui->combo_play_mod->setMainButton(mainBtn_play_mod);
    {
        QButtonGroup* group=new QButtonGroup(this);
        QRadioButton* mod1=new QRadioButton("顺序播放",this);
        QRadioButton* mod2=new QRadioButton("随机播放",this);
        QRadioButton* mod3=new QRadioButton("循环播放",this);
        QRadioButton* mod4=new QRadioButton("单个播放",this);//就是播完暂停
        QRadioButton* mod5=new QRadioButton("列表循环",this);
        mod1->setChecked(true);
        group->addButton(mod1);
        group->addButton(mod2);
        group->addButton(mod3);
        group->addButton(mod4);
        group->addButton(mod5);
        ui->widget_video->w_box->combo_mod->addWidget(mod1);
        ui->widget_video->w_box->combo_mod->addWidget(mod2);
        ui->widget_video->w_box->combo_mod->addWidget(mod3);
        ui->widget_video->w_box->combo_mod->addWidget(mod4);
        ui->widget_video->w_box->combo_mod->addWidget(mod5);
        connect(mod1,&QPushButton::clicked,this,[=](){
            ui->widget_video->w_box->combo_mod->btn_main->setIcon(QIcon(":res/mod_sequent.png"));
            play_mod=1;
            QString qstr=mod1->text();
            sendMsg(qstr);
        });
        connect(mod2,&QPushButton::clicked,this,[=](){
            ui->widget_video->w_box->combo_mod->btn_main->setIcon(QIcon(":res/mod_rand.png"));
            play_mod=2;
            QString qstr=mod2->text();
            sendMsg(qstr);
        });
        connect(mod3,&QPushButton::clicked,this,[=](){
            ui->widget_video->w_box->combo_mod->btn_main->setIcon(QIcon(":res/mod_loop.png"));
            play_mod=3;
            QString qstr=mod3->text();
            sendMsg(qstr);
        });
        connect(mod4,&QPushButton::clicked,this,[=](){
            ui->widget_video->w_box->combo_mod->btn_main->setIcon(QIcon(":res/mod_pause.png"));
            play_mod=4;
            QString qstr=mod4->text();
            sendMsg(qstr);
        });
        connect(mod5,&QPushButton::clicked,this,[=](){
            ui->widget_video->w_box->combo_mod->btn_main->setIcon(QIcon(":res/mod_list.png"));
            play_mod=5;
            QString qstr=mod5->text();
            sendMsg(qstr);
        });
    }
    // QPushButton* mainBtn_volume=new QPushButton("音量",this);
    // mainBtn_volume->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    // ui->combo_volume->setMainButton(mainBtn_volume);
    {
        QSlider* slider=new QSlider(Qt::Vertical,this);
        slider->setValue(50);
        audio->setVolume(slider->value());
        ui->widget_video->w_box->combo_volume->addWidget(slider);
        connect(slider,&QSlider::valueChanged,this,[=](int value){
            audio->setVolume(value);
            //sendMsg("音量:"+QString::number(slider->value()));
        });
        connect(slider,&QSlider::sliderReleased,this,[=](){
            sendMsg("音量:"+QString::number(slider->value()));
        });
    }

    connect(ui->btn_open,&QPushButton::clicked,this,[=](){
        QStringList list=QFileDialog::getOpenFileNames(this,"打开视频文件");
        //this->list_media.append(list);
        for(QString& qstr:list){
            //QString name=qstr.split("/").last();
            addFile(qstr);
        }
    });

    connect(ui->widget_video->w_box->btn_play,&QPushButton::clicked,this,[=](){
        QString text=ui->widget_video->w_box->btn_play->text();
        if(text=="暂停"){
            player->pause();
            ui->widget_video->w_box->btn_play->setText("播放");
            ui->widget_video->w_box->btn_play->setIcon(QIcon(":/res/start.png"));
        }else if(text=="播放"){
            player->play();
            ui->widget_video->w_box->btn_play->setText("暂停");
            ui->widget_video->w_box->btn_play->setIcon(QIcon(":/res/pause.png"));
        }
    });

    connect(ui->widget_video->w_box->btn_last,&QPushButton::clicked,this,[=](){
        this->setIndex(index_list_media--);
        player->setSource(QUrl::fromLocalFile(list_media[index_list_media]));
        player->play();
    });

    connect(ui->widget_video->w_box->btn_next,&QPushButton::clicked,this,[=](){
        player->setPosition(player->duration());
    });

    connect(ui->widget_video->w_box->btn_full,&QPushButton::clicked,this,[=](){
        const bool isPlaying=player->isPlaying();
        if(isPlaying){
            emit ui->widget_video->w_box->btn_play->clicked();
        }
        player->setVideoOutput(nullptr);
        ui->stackedWidget->setCurrentIndex(1);
        // QTimer::singleShot(50,this,[=](){

        // });
        player->setVideoOutput(ui->widget_video_2);
        if(isPlaying){
            player->play();
        }
        sendMsg("按下ESC键退出全屏");
    });

    connect(player,&QMediaPlayer::mediaStatusChanged,this,[=](int state){
        if(state==QMediaPlayer::EndOfMedia){
            //qDebug()<<"play_mod"<<play_mod;
            qDebug()<<"index_list_media"<<index_list_media;
            //qDebug()<<"list_media.size()"<<list_media.size();
            switch (play_mod) {
            case 1:{//顺序播放
                this->setIndex(index_list_media+1);
                player->setSource(QUrl::fromLocalFile(list_media[index_list_media]));
                player->play();
                break;
            }
            case 2:{//随机播放
                QRandomGenerator* generator=QRandomGenerator::global();
                this->setIndex( generator->bounded(0, list_media.size() ) );
                player->setSource(QUrl::fromLocalFile(list_media[index_list_media]));
                player->play();
                break;
            }
            case 3:{//循环播放
                player->play();
                break;
            }
            case 4:{//播完暂停
                ui->widget_video->w_box->btn_play->setText("播放");
                ui->widget_video->w_box->btn_play->setIcon(QIcon(":/res/pause.png"));
                break;
            }
            default:
                break;
            }
        }
    });


    QTimer::singleShot(10,this,[=](){
        // for(QObject* oj:this->children()){
        //     QWidget* w=dynamic_cast<QWidget*>(oj);
        //     if(w){
        //         w->setAcceptDrops(false);
        //         qDebug()<<"set do not drops";
        //     }
        // }
        ui->scrollArea->acceptDrops();
        ui->widget_scroll->acceptDrops();
        ui->widget_video->acceptDrops();
        ui->widget_video->installEventFilter(this);
        //ui->scrollArea->installEventFilter(this);
        //ui->widget_scroll->installEventFilter(this);
    });

    this->acceptDrops();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key()==Qt::Key_Escape && ui->stackedWidget->currentIndex()==1){
        const bool isPlaying=player->isPlaying();
        if(isPlaying){
            emit ui->widget_video->w_box->btn_play->clicked();
        }
        player->setVideoOutput(nullptr);
        ui->stackedWidget->setCurrentIndex(0);
        player->setVideoOutput(ui->widget_video);
        if(isPlaying){
            player->play();
        }
    }
}

void Widget::dragEnterEvent(QDragEnterEvent *ev)
{
    qDebug()<<"drag";
    if(ev->mimeData()->hasUrls()){
        ev->acceptProposedAction();
    }else{
        qDebug()<<"unfind url";
    }

}

void Widget::dropEvent(QDropEvent *ev)
{
    QList<QUrl> urls=ev->mimeData()->urls();
    if(!urls.isEmpty()){
        for(QUrl& url:urls){
            addFile(url.toLocalFile());
        }
    }
}

bool Widget::eventFilter(QObject *obj, QEvent *ev)
{
    //qDebug()<<"type"<<ev->type();
    if(ev->type()==QEvent::DragEnter){
        QDragEnterEvent* drag=dynamic_cast<QDragEnterEvent*>(ev);
        qDebug()<<"filter:drag";
        if(drag->mimeData()->hasUrls()){
            drag->acceptProposedAction();
        }else{
            qDebug()<<"filter:unfind url";
        }
        drag->ignore();
        return false;
    }else{
        return QObject::eventFilter(obj,ev);
    }
}



void Widget::setIndex(int new_index)
{
    if(this->list_media.size()==0){
        return;
    }
    if(index_list_media!=new_index){
        if(new_index==-1 && index_list_media==0){//如果索引在最前且要减少1，则到表尾
            index_list_media=list_media.size()-1;
        }else if(new_index==list_media.size() && index_list_media==list_media.size()-1){//如果索引在最后且要加1，则到表头
            index_list_media=0;
        }else if(new_index>=0 && new_index<list_media.size()){
            index_list_media=new_index;
        }else{
            qDebug()<<"index_list_media索引越界";
        }
    }
}

void Widget::sendMsg(QString msg)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    static QLabel* label=nullptr;
    if(label){
        delete label;
        label=nullptr;
    }else{
        //qDebug()<<"label为空";
    }
    label=new QLabel(msg);
    label->setWindowFlag(Qt::FramelessWindowHint);
    label->setStyleSheet("QLabel{"
                         "color:black;"
                         "font-size:20px;"
                         "font-family:微软雅黑;"
                         "}");
    label->setAlignment(Qt::AlignCenter);
    label->raise();
    label->show();
    QPoint pos=mapToGlobal( QPoint(this->width()/2-label->width()/2,this->height()/2-label->height()/2) );
    //QPoint pos(this->width()/2-label->width()/2,this->height()/2-label->height()/2);
    //qDebug()<<"pos"<<pos;
    label->move(pos);
    QTimer::singleShot(400,this,[=](){
        QVariantAnimation* animation=new QVariantAnimation(this);
        animation->setDuration(400);
        animation->setStartValue(0);
        animation->setEndValue(100);
        animation->start(QVariantAnimation::DeleteWhenStopped);
        connect(animation,&QVariantAnimation::valueChanged,this,[=](const QVariant& value){
            QPalette p=label->palette();
            p.setColor(QPalette::Base,QColor(0,0,0,value.toInt()));
            label->setPalette(p);
        });
        connect(animation,&QVariantAnimation::finished,this,[=](){
            label->deleteLater();
            label=nullptr;
        });
    });
}

void Widget::addFile(QString filename)
{
    this->list_media.append(filename);
    VideoButton* btn_view=new VideoButton(ui->widget_scroll);
    btn_view->setSourse(filename);
    btn_view->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
    btn_view->setMinimumSize(0,this->height()/6);
    btn_view->setStyleSheet("QPushButton{"
                            "text-align:left;"
                            "font-size:15px;"
                            "}");
    btn_view->show();
    QList<QLayoutItem*> list_todelete;
    for(int i=0;i<ui->layout_scroll->count();i++){
        QLayoutItem* item_todelete=ui->layout_scroll->itemAt(i);
        QSpacerItem* spacer=dynamic_cast<QSpacerItem*>(item_todelete);
        if(spacer){
            list_todelete.append(spacer);
        }
    }
    for(QLayoutItem* item:list_todelete){
        ui->layout_scroll->removeItem(item);
        //int num=0;
        //qDebug()<<"remove"<<num;
        delete item;
    }
    ui->layout_scroll->addWidget(btn_view);
    ui->layout_scroll->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Ignored,QSizePolicy::Expanding));
    //qDebug()<<"btn_view"<<btn_view->geometry();
    //qDebug()<<"btn_view"<<btn_view->isVisible();
    connect(btn_view,&VideoButton::clicked,this,[=](){
        //qDebug()<<"start main video";
        setIndex(list_media.indexOf(filename));
        player->setSource(QUrl::fromLocalFile(filename));
        player->play();
        qDebug()<<"main url:"<<QUrl::fromLocalFile(filename);
        qDebug()<<"player isplaying"<<player->isPlaying();
        ui->widget_video->w_box->btn_play->setText("暂停");
        ui->widget_video->w_box->btn_play->setIcon(QIcon(":/res/pause.png"));
    });
}
