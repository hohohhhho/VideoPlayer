#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videobutton.h"
#include "networkwidget.h"
#include "userlistwidget.h"

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
#include <QTime>
#include <QWindow>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPainter>
#include <QFontDialog>
#include <QAudioFormat>
#include <QAudioSource>
#include <QMediaDevices>
#include <QMovie>
#include <QProcess>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QHttpPart>
#include <QMessageAuthenticationCode>

#define EMOTION_ANALYSIS_INTERVAL 100//情绪识别间隔

inline const QString currentItemPath = "D:/p1/QtProject/VideoPlayer";

QMutex mutex_user_list,mutex_frame;
QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists;//用户的视频列表
/*QVector<↓>
        QPair<QString,↓>//每一对代表一个用户列表
                    QList<↓>//每一个都是代表所在用户列表的子视频
                        QPair<QString,QString>//名称和路径组成一个子视频
*/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("视频播放器");
    this->setWindowFlag(Qt::FramelessWindowHint);

    this->player=new QMediaPlayer(this);
    this->sink=new QVideoSink(this);
    this->dlg_emotion=nullptr;
    this->thread_emo=nullptr;
    this->worker_emo=nullptr;
    this->manager=new QNetworkAccessManager(this);

    this->dir_save_history=QDir::currentPath()+"/data/history";
    ui->set_widget->edit_save_file->setText(dir_save_history);
    changeBackgroundkPixmap(":res/background.png");

    QAudioOutput* audio=new QAudioOutput(this);


    player->setAudioOutput(audio);
    player->setVideoSink(sink);

    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget_2->setCurrentIndex(0);

    ui->btn_network->setVisible(false);
    ui->btn_tool->setVisible(false);
    ui->btn_list->setVisible(false);
    ui->btn_open->setVisible(false);
    ui->stackedWidget_2->setStyleSheet("QWidget{"
                                       "background-color:transparent;"
                                       "}"
                                       "QStackWidget{"
                                       "background-color:transparent;"
                                       "}"
                                       "QPushButton{"
                                       "background-color:lightgray;"
                                       "}");
    ui->scrollArea_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_set->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_history->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_ai->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(sink, &QVideoSink::videoFrameChanged,this,[=](const QVideoFrame &frame){
        if(thread_emo){
            mutex_frame.lock();
            QVideoFrame cloneframe=frame;
            mutex_frame.unlock();
            ui->widget_video->video_widget->setFrame(cloneframe);
        }else{
            ui->widget_video->video_widget->setFrame(frame);
        }
    });
    // //ai模块
    {
        QPushButton* btn_emo=new QPushButton("情绪识别",ui->scrollAreaWidgetContents_ai);
        QPushButton* btn_ffmpeg=new QPushButton("ffmpeg",ui->scrollAreaWidgetContents_ai);
        QPushButton* btn_srt=new QPushButton("设置srt字幕",ui->scrollAreaWidgetContents_ai);
        QVBoxLayout* layout=dynamic_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents_ai->layout());
        btn_emo->setCheckable(true);
        if(layout){
            layout->insertWidget(0,btn_emo);
            layout->insertWidget(1,btn_ffmpeg);
            layout->insertWidget(2,btn_srt);
        }
        connect(btn_emo,&QPushButton::clicked,this,[=](){
            if(!btn_emo->isChecked()){
                if(dlg_emotion){
                    dlg_emotion->setVisible(false);
                }
                return;
            }
            if(dlg_emotion){
                delete dlg_emotion;
                dlg_emotion=nullptr;
            }
            if(thread_emo){
                thread_emo->quit();
                thread_emo->wait();
                delete thread_emo;
                thread_emo=nullptr;
            }
            if(worker_emo){
                delete worker_emo;
                worker_emo=nullptr;
            }
            // mutex_dlg_emotion.unlock();
            // mutex_dlg_emotion.lock();
            this->thread_emo = new QThread(this);
            this->worker_emo = new EmotionAnalysisWorker();
            this->dlg_emotion=new QDialog(this);
            dlg_emotion->setWindowFlag(Qt::FramelessWindowHint);
            dlg_emotion->show();
            //给情绪识别的线程传递帧信息
            connect(sink, &QVideoSink::videoFrameChanged, worker_emo, [=](const QVideoFrame &frame){
                static QElapsedTimer etimer;
                if(!etimer.isValid()){
                    etimer.start();
                }
                int interval=etimer.elapsed();
                if(interval >= EMOTION_ANALYSIS_INTERVAL){//控制帧率
                    etimer.restart();
                    mutex_frame.lock();
                    QVideoFrame cloneframe = frame;
                    mutex_frame.unlock();
                    worker_emo->processFrame(cloneframe, ui->widget_video->video_widget->size());
                }
            },Qt::QueuedConnection);

            //将识别到的信息传递回来显示在ui上
            connect(worker_emo, &EmotionAnalysisWorker::analysisCompleted,this,[=](QList<QRect> faces, QHash<QRect, QString> emotions){
                mutex_dlg_emotion.lock();
                if(dlg_emotion){
                    current_faces = std::move(faces);
                    current_emotions = std::move(emotions);

                    QBitmap mask(dlg_emotion->size());
                    mask.fill(Qt::color0);
                    QPainter painter(&mask);
                    painter.setPen(QPen(Qt::color1,2));
                    painter.setBrush(Qt::color0);
                    painter.drawPoint(0,0);
                    for(QRect& rect:current_faces){
                        painter.drawRect(rect);
                        painter.drawText(rect,current_emotions[rect]);
                    }

                    QPoint globalPos = ui->widget_video->video_widget->mapToGlobal(QPoint(0, 0));
                    dlg_emotion->move(globalPos);
                    dlg_emotion->resize(ui->widget_video->video_widget->size());
                    dlg_emotion->setMask(mask);
                }
                mutex_dlg_emotion.unlock();
            },Qt::DirectConnection);

            worker_emo->moveToThread(thread_emo);
            thread_emo->start();
        });
        connect(btn_ffmpeg,&QPushButton::clicked,this,[=](){

        });
        connect(btn_srt,&QPushButton::clicked,this,[=](){
            QString filename=QFileDialog::getOpenFileName(this);
            if(QFileInfo(filename).suffix()==".srt"){
                addSrt(filename);
            }

        });
    }
    //倍速
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
    //播放模式
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
    //音量
    {
        QWidget* w_volume=new QWidget(this);
        QVBoxLayout* layout=new QVBoxLayout(w_volume);
        QLabel* label=new QLabel("20",this);
        QSlider* slider=new QSlider(Qt::Vertical,this);

        layout->addWidget(label,1);
        layout->addWidget(slider,9);

        slider->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
        slider->setValue(20);
        audio->setVolume(static_cast<float>(slider->value())/100);
        label->setAlignment(Qt::AlignCenter);
        ui->widget_video->w_box->combo_volume->addWidget(w_volume);

        connect(slider,&QSlider::valueChanged,this,[=](int value){
            audio->setVolume(static_cast<float>(value)/100);
            label->setText(QString::number(value));
            //sendMsg("音量:"+QString::number(slider->value()));
        });
        connect(slider,&QSlider::sliderReleased,this,[=](){
            sendMsg("音量:"+QString::number(slider->value()));
        });
    }

    connect(player,&QMediaPlayer::playingChanged,this,[=](bool isPlaying){
        if(isPlaying){
            ui->widget_video->w_box->btn_play->setText("暂停");
            ui->widget_video->w_box->btn_play->setIcon(QIcon(":/res/pause.png"));
            ui->widget_music->btn_play->setText("暂停");
            ui->widget_music->btn_play->setIcon(QIcon(":/res/pause.png"));
        }else{
            ui->widget_video->w_box->btn_play->setText("播放");
            ui->widget_video->w_box->btn_play->setIcon(QIcon(":/res/start.png"));
            ui->widget_music->btn_play->setText("播放");
            ui->widget_music->btn_play->setIcon(QIcon(":/res/start.png"));
        }
    });

    connect(ui->btn_open,&QPushButton::clicked,this,[=](){
        QStringList list=QFileDialog::getOpenFileNames(this,"打开视频文件");
        //this->list_media.append(list);
        for(QString& qstr:list){
            //QString name=qstr.split("/").last();
            addFile(qstr);
        }
        loadNextVideo();
    });

    connect(ui->widget_video->w_box->btn_play,&QPushButton::clicked,this,[=](){
        QString text=ui->widget_video->w_box->btn_play->text();
        if(text=="暂停"){
            player->pause();
        }else if(text=="播放"){
            player->play();
        }
    });

    connect(ui->widget_video->w_box->btn_last,&QPushButton::clicked,this,[=](){
        this->setIndex(index_list_media-1);
        if(this->list_media.size()>0){
            updateSource(list_media[index_list_media]);
            ui->groupBox->setTitle(list_media[index_list_media].split("/").last());
            player->play();
        }
    });

    connect(ui->widget_video->w_box->btn_next,&QPushButton::clicked,this,[=](){
        player->setPosition(player->duration());
    });

    connect(ui->widget_video->w_box->btn_full,&QPushButton::clicked,this,[=](){
        fillScreen();
    });

    connect(ui->widget_music->btn_play,&QPushButton::clicked,this,[=](){
        QString text=ui->widget_music->btn_play->text();
        if(text=="暂停"){
            player->pause();
        }else if(text=="播放"){
            player->play();
        }
    });

    connect(ui->widget_music->btn_last,&QPushButton::clicked,this,[=](){
        this->setIndex(index_list_media-1);
        if(this->list_media.size()>0){
            updateSource(list_media[index_list_media]);
            ui->groupBox->setTitle(list_media[index_list_media].split("/").last());
            player->play();
        }
    });

    connect(ui->widget_music->btn_next,&QPushButton::clicked,this,[=](){
        player->setPosition(player->duration());
    });

    connect(player,&QMediaPlayer::mediaStatusChanged,this,[=](int state){
        if(state==QMediaPlayer::EndOfMedia){
            //qDebug()<<"play_mod"<<play_mod;
            qDebug()<<"index_list_media"<<index_list_media;
            //qDebug()<<"list_media.size()"<<list_media.size();
            switch (play_mod) {
            case 1:{//顺序播放
                this->setIndex(index_list_media+1);
                if(this->list_media.size()>0)
                    updateSource(list_media[index_list_media]);
                player->play();
                break;
            }
            case 2:{//随机播放
                QRandomGenerator* generator=QRandomGenerator::global();
                this->setIndex( generator->bounded( 0, list_media.size() ) );
                if(this->list_media.size()>0)
                    updateSource(list_media[index_list_media]);
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

    connect(player,&QMediaPlayer::durationChanged,this,[=](int duration){
        QStringList list=ui->widget_video->w_box->label_progress->text().split("/");
        QString current_position=list.first();//取当前进度，保留
        QTime new_duration(0,0,0);
        new_duration=new_duration.addMSecs(duration);
        current_position.append("/").append(new_duration.toString());
        //qDebug()<<"list"<<list<<"current_position"<<current_position<<"new_duration"<<new_duration;

        ui->widget_video->w_box->label_progress->setText(current_position);
    });

    connect(player,&QMediaPlayer::positionChanged,this,[=](int position){
        QStringList list=ui->widget_video->w_box->label_progress->text().split("/");
        QString current_duration=list.last();//取当前最大时长，保留
        QTime new_position(0,0,0);
        new_position=new_position.addMSecs(position);
        QString qstr_label;
        qstr_label=new_position.toString()+"/"+current_duration;
        ui->widget_video->w_box->label_progress->setText(qstr_label);

        double progress=static_cast<double>(position)/player->duration();
        if(ui->stack_video->currentWidget()==ui->page_video){
            ui->widget_video->updateCurrentTime(new_position);
            ui->widget_video->bar->setProgress(progress);
        }else if(ui->stack_video->currentWidget()==ui->page_gif){
            ui->widget_music->setProgress(progress);
        }
    });

    connect(ui->widget_video->bar,&ProgressBar::changedProgress,this,[=](double progress){
        player->setPosition(progress*player->duration());
    });

    connect(ui->widget_music,&MusicWidget::progessChanged,this,[=](float progress){
        player->setPosition(progress*player->duration());
    });


    connect(ui->btn_set,&QPushButton::clicked,this,[=](){
        if(ui->btn_set->isChecked()){
            ui->stackedWidget_2->setCurrentWidget(ui->page_set);
            ui->btn_history->setChecked(false);
            ui->btn_ai->setChecked(false);
        }else{
            ui->stackedWidget_2->setCurrentIndex(0);
        }
    });
    connect(ui->btn_history,&QPushButton::clicked,this,[=](){
        if(ui->btn_history->isChecked()){
            ui->stackedWidget_2->setCurrentWidget(ui->page_history);
            ui->btn_set->setChecked(false);
            ui->btn_ai->setChecked(false);
            for(QObject* oj:ui->scrollAreaWidgetContents_history->children()){
                VideoButton* btn=dynamic_cast<VideoButton*>(oj);

                if(btn){
                    // qDebug()<<"btn";
                    // QMutexLocker locker(&btn->mutex_loaded);
                    // btn->mutex_loaded.lock();
                    bool loaded=btn->isLoaded();
                    // btn->mutex_loaded.unlock();
                    if(!loaded){
                        // btn->playSource();
                        queue_load_video_history.append(btn);
                        static bool first_time=true;
                        if(first_time){
                            first_time=false;
                            loadingVideo=true;
                            loadNextHistoryVideo();

                        }
                    }
                }

            }
        }else{
            ui->stackedWidget_2->setCurrentIndex(0);
        }
    });
    connect(ui->btn_ai,&QPushButton::clicked,this,[=](){
        if(ui->btn_ai->isChecked()){
            ui->stackedWidget_2->setCurrentWidget(ui->page_ai);
            ui->btn_set->setChecked(false);
            ui->btn_history->setChecked(false);
        }else{
            ui->stackedWidget_2->setCurrentIndex(0);
        }
    });
    connect(ui->btn_more,&QPushButton::clicked,this,[=](){
        bool isVisible=ui->btn_network->isVisible();
        if(isVisible){
            ui->vlayout_view->setStretch(1,0);
            ui->vlayout_view->setStretch(2,16);
        }else{
            ui->vlayout_view->setStretch(1,1);
            ui->vlayout_view->setStretch(2,15);
        }
        ui->btn_network->setVisible(!isVisible);
        ui->btn_tool->setVisible(!isVisible);
        ui->btn_list->setVisible(!isVisible);
        ui->btn_open->setVisible(!isVisible);
    });
    connect(ui->btn_network,&QPushButton::clicked,this,[=](){
        NetworkWidget* w=new NetworkWidget;
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });
    connect(ui->btn_list,&QPushButton::clicked,this,[=](){
        UserListWidget* w=new UserListWidget;
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->init();
        connect(w,&UserListWidget::playVideo,this,[=](const QString& source){
            this->updateSource(source);
            player->play();
        });
        connect(w,&UserListWidget::addToCurrentList,this,[=](const QString& source){
            this->addFile(source);
            loadNextVideo();
        });
        w->show();
    });

    connect(ui->btn_seek,&QPushButton::clicked,this,[=](){
        QString key=ui->edit_seek->text();

        for(QObject* oj:ui->scrollAreaWidgetContents_history->children()){
            VideoButton* btn=dynamic_cast<VideoButton*>(oj);
            if(btn){
                QString name=btn->getSource().split("/").last().split(".").first();
                if(name.contains(key)){//包含关键字则显示，否则隐藏
                    btn->setVisible(true);
                }else{
                    btn->setVisible(false);
                }
            }
        }
    });

    connect(ui->edit_seek,&QLineEdit::returnPressed,this,[=](){
        emit ui->btn_seek->clicked();
    });

    connect(ui->btn_clear,&QPushButton::clicked,this,[=](){
        if(QMessageBox::warning(this,"警告","确定要删除当前列表吗?"
                                 ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
                                                        ==QMessageBox::StandardButton(QMessageBox::Yes)){
            for(QObject* oj:ui->scrollAreaWidgetContents_history->children()){
                VideoButton* btn=dynamic_cast<VideoButton*>(oj);
                if(btn){
                    if(btn->isVisible()){
                        this->list_history.removeAll(btn->getSource());
                        btn->deleteLater();
                    }
                }
            }
        }
    });

    connect(ui->head_widget,&HeadButton::clicked,this,[=](int index_btn){
        if(index_btn==0){
            static bool top=true;
            this->setWindowFlag(Qt::WindowStaysOnTopHint,top);
            this->show();
            top=!top;
        }else if(index_btn==1){
            QPoint pos=mapFromGlobal(QCursor::pos());
            this->animation_mask=new QVariantAnimation(this);
            animation_mask->setStartValue(1);
            int endvalue=std::sqrt( (width()*width()+height()*height()) );
            animation_mask->setEndValue(endvalue);

            animation_mask->setDuration(300);
            connect(animation_mask,&QVariantAnimation::valueChanged,this,[=](const QVariant& value){
                QBitmap mask(this->size());
                mask.fill(Qt::color1);
                QPainter pmask(&mask);
                pmask.setPen(Qt::color0);
                pmask.setBrush(Qt::color0);
                pmask.drawEllipse(pos,value.toInt(),value.toInt());
                this->setMask(mask);
                if(value.toInt()>animation_mask->endValue().toInt()*0.9){
                    this->showMinimized();
                }
            });
            connect(animation_mask,&QVariantAnimation::finished,this,[=](){
                qDebug()<<"finished";
            });
            animation_mask->start();

        }else if(index_btn==2){
            if(isMaximized()){
                showNormal();
            }else{
                showMaximized();
            }
        }else if(index_btn==3){
            this->close();
        }
    });

    connect(ui->set_widget->combo_theme, &QComboBox::currentTextChanged, this, [=](QString text){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        if(text=="深色"){
            ui->head_widget->changeTheme(false);
            ui->widget_video->w_box->changeTheme(false);
            this->isWhite=false;
            QPalette palette;
            // 基础配色
            static const int num1=93;
            palette.setColor(QPalette::Window, QColor(num1, num1, num1));
            palette.setColor(QPalette::WindowText, Qt::white);
            palette.setColor(QPalette::Base, QColor(35, 35, 35));
            palette.setColor(QPalette::AlternateBase, QColor(num1, num1, num1));
            // 文本
            palette.setColor(QPalette::Text, Qt::white);
            palette.setColor(QPalette::PlaceholderText, QColor(127, 127, 127));
            // 按钮
            palette.setColor(QPalette::Button, QColor(num1, num1, num1));
            palette.setColor(QPalette::ButtonText, Qt::white);
            palette.setColor(QPalette::BrightText, Qt::red);
            // 交互状态
            palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            palette.setColor(QPalette::HighlightedText, Qt::white);
            palette.setColor(QPalette::Link, QColor(42, 130, 218));
            // 禁用状态
            palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
            palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
            // 其他元素
            palette.setColor(QPalette::ToolTipBase, Qt::white);
            palette.setColor(QPalette::ToolTipText, Qt::black);
            QApplication::setPalette(palette);
            ui->btn_ai->repaint();
            update();
        }else if(text=="浅色"){
            ui->head_widget->changeTheme(true);
            ui->widget_video->w_box->changeTheme(true);
            this->isWhite=true;

            QPalette palette;
            palette.setColor(QPalette::Window, Qt::white);
            palette.setColor(QPalette::WindowText, QColor(53, 53, 53));
            palette.setColor(QPalette::Base, Qt::white);
            palette.setColor(QPalette::AlternateBase, QColor(220, 220, 220));
            // 文本
            palette.setColor(QPalette::Text, QColor(53, 53, 53));
            palette.setColor(QPalette::PlaceholderText, QColor(160, 160, 160));
            // 按钮
            palette.setColor(QPalette::Button, QColor(240, 240, 240));
            palette.setColor(QPalette::ButtonText, QColor(53, 53, 53));
            palette.setColor(QPalette::BrightText, Qt::red);
            // 交互状态
            palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            palette.setColor(QPalette::HighlightedText, Qt::white);
            palette.setColor(QPalette::Link, QColor(0, 122, 204));
            // 禁用状态
            palette.setColor(QPalette::Disabled, QPalette::Text, QColor(160, 160, 160));
            palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(160, 160, 160));
            // 其他元素
            palette.setColor(QPalette::ToolTipBase, Qt::white);
            palette.setColor(QPalette::ToolTipText, Qt::black);
            QApplication::setPalette(palette);
            update();
        }
    });

    connect(ui->set_widget->btn_choose_font,&QPushButton::clicked,this,[=](){
        QFontDialog* dlg=new QFontDialog(this);
        dlg->setCurrentFont(/*QApplication::font()*/QFont(font_family));
        if(dlg->exec()==QDialog::Accepted){
            if(ui->set_widget->isVisible()) user_setting_exchanged = true;
            QFont font=dlg->currentFont();
            changeFont(font.family());
        }
        dlg->deleteLater();
    });

    connect(ui->set_widget->btn_open_file,&QPushButton::clicked,this,[=](){
        QUrl url=QFileDialog::getExistingDirectoryUrl(this,"选择路径");
        if(url.isValid()){
            if(QMessageBox::information(this,"提示","确定更改历史记录保存路径?"
                                     ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
                ==QMessageBox::StandardButton(QMessageBox::Yes)){

                if(ui->set_widget->isVisible()) user_setting_exchanged = true;
                ui->set_widget->edit_save_file->setText(url.toLocalFile());
                this->dir_save_history=url.toLocalFile();
            }
        }
    });

    connect(ui->set_widget->spin_max_history_num,&QSpinBox::valueChanged,this,[=](int value){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        this->max_history_num=value;
    });

    connect(ui->set_widget->btn_clear_user_data,&QPushButton::clicked,this,[=](){
        if(QMessageBox::warning(this,"提示","确定清除用户数据?"
                                     ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
            ==QMessageBox::StandardButton(QMessageBox::Yes)){

            if(ui->set_widget->isVisible()) user_setting_exchanged = true;
            QDir dir=QDir::currentPath()+"/data";
            qDebug()<<"dir.path()"<<dir.path();
            deleteFilesFromPath(dir.path());
            QMessageBox::information(this,"提示","清理完成!");
        }
    });

    connect(ui->set_widget->spin_max_num_load_video, &QSpinBox::valueChanged, this, [=](int value){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        mutex_max_num_load_video_once.lock();
        this->max_num_load_video_once=value;
        mutex_max_num_load_video_once.unlock();
    });

    connect(ui->set_widget->spin_interval_load_video, &QSpinBox::valueChanged, this, [=](int value){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        mutex_interval_load_video.lock();
        this->interval_load_video=value;
        mutex_interval_load_video.unlock();
    });

    connect(ui->set_widget->spin_lumilance, &QSpinBox::valueChanged, this, [=](int value){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        double lumilance=static_cast<double>(value)/100.0;
        this->min_lumilance=lumilance;
        for(QObject* oj:ui->scrollAreaWidgetContents_list->children()){
            VideoButton* btn=dynamic_cast<VideoButton*>(oj);
            if(btn){
                btn->setMinLumilance(lumilance);
            }
        }
        for(QObject* oj:ui->scrollAreaWidgetContents_history->children()){
            VideoButton* btn=dynamic_cast<VideoButton*>(oj);
            if(btn){
                btn->setMinLumilance(lumilance);
            }
        }
    });

    connect(ui->set_widget->spin_forward,&QSpinBox::valueChanged,this,[=](int value){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        this->forward_sec=value;
    });

    connect(ui->set_widget->spin_backward,&QSpinBox::valueChanged,this,[=](int value){
        if(ui->set_widget->isVisible()) user_setting_exchanged = true;
        this->backward_sec=value;
    });

    connect(ui->set_widget->edit_pxp_path,&QLineEdit::returnPressed,this,[=](){
        if(QMessageBox::information(this,"提示","确定更改背景图片?"
                                     ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
            ==QMessageBox::StandardButton(QMessageBox::Yes)){

            if(ui->set_widget->isVisible()) user_setting_exchanged = true;
            changeBackgroundkPixmap(ui->set_widget->edit_pxp_path->text());
        }
    });

    connect(ui->set_widget->btn_open_file_pxp,&QPushButton::clicked,this,[=](){
        QString filename=QFileDialog::getOpenFileName(this,"选择路径");
        if(!filename.isEmpty()){
            if(QMessageBox::information(this,"提示","确定更改背景图片?"
                                         ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
                ==QMessageBox::StandardButton(QMessageBox::Yes)){

                if(ui->set_widget->isVisible()) user_setting_exchanged = true;
                changeBackgroundkPixmap(filename);
            }
        }
    });

    connect(ui->set_widget->btn_default,&QPushButton::clicked,this,[=](){
        if(QMessageBox::information(this,"提示","确定要恢复默认吗?"
                                     ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
            ==QMessageBox::StandardButton(QMessageBox::Yes)){

            if(ui->set_widget->isVisible()) user_setting_exchanged = true;
            QString path(":/res/background.png");
            changeBackgroundkPixmap(path);
        }
    });

    connect(ui->set_widget->btn_save_all,&QPushButton::clicked,this,[=](){
        if(QMessageBox::question(this,"警告","确定保存当前操作吗?"
                                  ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))
            ==QMessageBox::StandardButton(QMessageBox::Yes)){

            user_setting_exchanged = false;
            saveUserInfo();
        }
    });

    QTimer::singleShot(100,this,[=](){
        user_setting_exchanged = false;
        loadUserInfo();
        loadHistoryFromfile();
    });
}

MainWindow::~MainWindow()
{
    player->stop();

    if(thread_emo){
        thread_emo->quit();
        thread_emo->wait();
        delete thread_emo;
    }
    // thread_whisper->quit();
    // thread_whisper->wait();
    // delete thread_whisper;

    // delete ringbuffer;
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);
    QRect titleRect = ui->head_widget->geometry();
    painter.fillRect(titleRect, Qt::transparent);

    if(!pxp_background.isNull()){
        painter.drawPixmap(0, titleRect.bottom(), pxp_background);
    }else{
        qDebug()<<"pxp_background is null";
    }
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    Q_UNUSED(ev);
    if(ev->key()==Qt::Key_Escape && ui->stackedWidget->currentIndex()==1){
        escScreen();
    }else if(ev->key()==Qt::Key_Space){
        qDebug()<<"space";
        if(this->player->isPlaying()){
            player->pause();
        }else{
            player->play();
        }
    }else if(ev->key()==Qt::Key_Left){
        int sec=qBound(0,player->position()-backward_sec*1000,player->duration());
        sendMsg("快退"+QString::number(backward_sec)+"秒");
        player->setPosition(sec);
    }else if(ev->key()==Qt::Key_Right){
        int sec=qBound(0,player->position()+forward_sec*1000,player->duration());
        sendMsg("快进"+QString::number(forward_sec)+"秒");
        player->setPosition(sec);
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    if(ui->stackedWidget->currentIndex()==0){
        fillScreen();
    }else{
        escScreen();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    //qDebug()<<"drag";
    if(ev->mimeData()->hasUrls()){
        ev->acceptProposedAction();
    }else{
        qDebug()<<"unfind url";
    }

}

void MainWindow::dropEvent(QDropEvent *ev)
{
    const static QString suffix_file="mp4 avi mkv mp3 wav ogg";
    const static QString suffix_srt=".srt";
    QList<QUrl> urls=ev->mimeData()->urls();
    if(!urls.isEmpty()){
        for(QUrl& url:urls){
            QString filename(url.toLocalFile());
            QFileInfo fileinfo(filename);
            if(suffix_file.contains(fileinfo.suffix())){
                addFile(filename);
            }else if(suffix_srt.contains(fileinfo.suffix())){
                addSrt(filename);
            }else{
                QMessageBox::information(this,"提示,","不支持"+fileinfo.suffix()+"格式");
            }
        }
        loadNextVideo();
    }

}

void MainWindow::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);

    pxp_background=pxp_background.scaled(ev->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    QScreen* screen=this->screen();
    //qreal ratio=QApplication::primaryScreen()->devicePixelRatio();
    if(screen->size()!=ev->size()){
        this->current_size=ev->size();

    }
    update();
    // qDebug()<<"size"<<this->current_size;

    // for(QObject* oj:ui->scrollAreaWidgetContents1->children()){
    //     VideoButton* btn=dynamic_cast<VideoButton*>(oj);
    //     if(btn){
    //         btn->loadsource(btn->getsource());
    //         btn->playsource();
    //     }
    // }
    // for(QObject* oj:ui->scrollAreaWidgetContents3->children()){
    //     VideoButton* btn=dynamic_cast<VideoButton*>(oj);
    //     if(btn){
    //         btn->loadsource(btn->getsource());
    //         btn->playsource();
    //     }
    // }
}

void MainWindow::moveEvent(QMoveEvent *ev)
{
    if(ev->pos()!=QPoint(0,0)){
        this->current_pos=ev->pos();
    }
    // qDebug()<<"pos"<<this->current_pos;
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button()==Qt::LeftButton){
        this->start_point=ev->globalPosition().toPoint();
        this->start_pos=this->pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *ev)
{
    if( (ev->buttons() & Qt::LeftButton) && ui->stackedWidget->currentWidget()==ui->page){
        QPoint dp=ev->globalPosition().toPoint()-this->start_point;
        this->move(start_pos+dp);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    this->start_point=this->start_pos=QPoint(0,0);
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if(user_setting_exchanged){
        QMessageBox::StandardButton standard=QMessageBox::question(this,"提示","需要保存用户设置吗?"
                                                                     ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel));
        if(standard==QMessageBox::StandardButton(QMessageBox::Yes)){
            saveUserInfo();
            ev->accept();
        }else if(standard==QMessageBox::StandardButton(QMessageBox::No)){
            ev->accept();
        }else{
            ev->ignore();
        }
    }
}

// bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
// {
//     if(ev->type()==QEvent::KeyPress){
//         // QKeyEvent* key_event=dynamic_cast<QKeyEvent*>(ev);
//         ev->ignore();
//         // key_event->ignore();
//         return false;
//     }else{
//         return QObject::eventFilter(obj,ev);
//     }
// }

// bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
// {
//     //qDebug()<<"type"<<ev->type();
//     if(ev->type()==QEvent::DragEnter){
//         QDragEnterEvent* drag=dynamic_cast<QDragEnterEvent*>(ev);
//         qDebug()<<"filter:drag";
//         if(drag->mimeData()->hasUrls()){
//             drag->acceptProposedAction();
//         }else{
//             qDebug()<<"filter:unfind url";
//         }
//         drag->ignore();
//         return false;
//     }else{
//         return QObject::eventFilter(obj,ev);
//     }
// }



void MainWindow::setIndex(int new_index)
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

void MainWindow::sendMsg(QString msg)
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

    QPoint video_center;
    if(ui->widget_video->video_widget==player->videoOutput()){
        video_center=QPoint(ui->widget_video->geometry().center()) - QPoint(label->width()/2,label->height()/2);
    }else{
        video_center=QPoint(ui->widget_video_2->geometry().center()) - QPoint(label->width()/2,label->height()/2);
    }
    // qDebug()<<"video_center"<<video_center;
    QPoint pos=mapToGlobal(video_center);
    //QPoint pos=mapToGlobal( QPoint(this->width()/2-label->width()/2,this->height()/2-label->height()/2) );
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

void MainWindow::addFile(QString filename)
{
    QFileInfo fileinfo(filename);
    QString suffix=fileinfo.suffix();
    const static QString suffix_video="mp4 avi mkv";
    const static QString suffix_music="mp3 wav ogg";
    bool isMusic=false;
    if(suffix_video.contains(suffix)){

    }else if(suffix_music.contains(suffix)){
        isMusic=true;
    }else{
        return;
    }
    for(QString& qstr:this->list_media){
        if(qstr==filename){
            return;
        }
    }

    this->list_media.append(filename);
    VideoButton* btn_view=new VideoButton(ui->scrollAreaWidgetContents_list);
    btn_view->loadSource(filename);
    if(isMusic){
        btn_view->setMusic(true);
    }else{
        queue_load_video.append(btn_view);
    }
    btn_view->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    btn_view->setFixedHeight(this->height()/6);
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
    connect(btn_view,&VideoButton::willDelete,this,[=](){
        btn_view->deleteLater();
    });
    connect(btn_view,&VideoButton::movePosition,this,[=](bool up){
        QWidget* parent=dynamic_cast<QWidget*>(btn_view->parent());
        if(parent){
            bool ishistory=false;
            if(parent==ui->scrollAreaWidgetContents_history){
                ishistory=true;
            }
            QVBoxLayout* layout=ishistory?ui->layout_scroll_history:ui->layout_scroll;
            if(layout){
                int index=layout->indexOf(btn_view);
                layout->removeWidget(btn_view);
                if(up){
                    layout->insertWidget(index-1,btn_view);
                }else{
                    layout->insertWidget(index+1,btn_view);
                }
                update();
            }else{
                qDebug()<<"layout类型转换失败";
            }
        }else{
            qDebug()<<"parent类型转换失败";
        }
    });
    connect(btn_view,&VideoButton::clicked,this,[=](){
        //qDebug()<<"start main video";
        setIndex(list_media.indexOf(filename));

        this->updateSource(filename);

        // player->setVideoOutput(ui->widget_video->video_widget);
        player->play();

        bool has_history=false;
        for(QObject* oj:ui->scrollAreaWidgetContents_history->children()){//当点击的某条视频存在历史观看记录，把这条历史记录移到最前面
            VideoButton* btn=dynamic_cast<VideoButton*>(oj);
            if(btn){
                if(btn->getSource()==filename){
                    // qDebug()<<"找到历史记录";
                    has_history=true;
                    ui->layout_scroll_history->removeWidget(btn);
                    ui->layout_scroll_history->insertWidget(0,btn);
                    update();
                    break;
                }
            }
        }

        if(!has_history){//没有找到历史记录则添加一个进去
            this->list_history.append(filename);
            VideoButton* btn_view_history=new VideoButton(ui->scrollAreaWidgetContents_history);
            btn_view_history->loadSource(filename);
            btn_view_history->setMusic(isMusic);
            btn_view_history->stamp=QDateTime::currentDateTime();
            btn_view_history->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
            btn_view_history->setFixedHeight(this->height()/6);
            btn_view_history->setStyleSheet("QPushButton{"
                                            "text-align:left;"
                                            "font-size:15px;"
                                            "}");
            btn_view_history->show();
            QList<QLayoutItem*> list_todelete_history;
            for(int i=0;i<ui->layout_scroll_history->count();i++){
                QLayoutItem* item_todelete=ui->layout_scroll_history->itemAt(i);
                QSpacerItem* spacer=dynamic_cast<QSpacerItem*>(item_todelete);
                if(spacer){
                    list_todelete_history.append(spacer);
                }
            }
            for(QLayoutItem* item:list_todelete_history){
                ui->layout_scroll_history->removeItem(item);
                delete item;
            }
            ui->layout_scroll_history->addWidget(btn_view_history);
            ui->layout_scroll_history->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Ignored,QSizePolicy::Expanding));
            connect(btn_view_history,&VideoButton::clicked,this,[=](){
                // setIndex(list_media.indexOf(filename));

                this->updateSource(filename);

                // player->setVideoOutput(ui->widget_video->video_widget);
                player->play();
            });
            connect(btn_view_history,&VideoButton::willDelete,this,[=](){
                btn_view_history->deleteLater();
            });
            connect(btn_view_history,&VideoButton::movePosition,this,[=](bool up){
                QWidget* parent=dynamic_cast<QWidget*>(btn_view_history->parent());
                if(parent){
                    bool ishistory=false;
                    if(parent==ui->scrollAreaWidgetContents_history){
                        ishistory=true;
                    }
                    QVBoxLayout* layout=ishistory?ui->layout_scroll_history:ui->layout_scroll;
                    if(layout){
                        int index=layout->indexOf(btn_view_history);
                        layout->removeWidget(btn_view_history);
                        if(up){
                            layout->insertWidget(index-1,btn_view_history);
                        }else{
                            layout->insertWidget(index+1,btn_view_history);
                        }
                        update();
                    }else{
                        qDebug()<<"layout类型转换失败";
                    }
                }else{
                    qDebug()<<"parent类型转换失败";
                }
            });

            QJsonObject jsonObject;
            jsonObject["name"] = btn_view_history->name;
            jsonObject["source"] = filename;
            jsonObject["timestamp"] = btn_view_history->stamp.toString();
            jsonObject["isMusic"] = btn_view_history->isMusicButton()?"true":"false";

            QDir dir_history_path=dir_save_history;
            if(!dir_history_path.exists()){
                bool result=dir_history_path.mkpath(dir_history_path.path());
                if(!result){
                    qDebug()<<"history路径创建失败";
                }
            }
            QString jsonname=btn_view_history->getSource().replace(":","").replace("/","_")+".json";

            QFile file(dir_history_path.filePath(jsonname));
            if (file.open(QIODevice::WriteOnly)) {
                QJsonDocument jsonDoc(jsonObject);
                file.write(jsonDoc.toJson());
                file.close();
            } else {
                qWarning("历史记录文件打开失败");
            }
        }
    });

    // timer_load_video->start();
}

void MainWindow::addSrt(QString filename)
{
    ui->widget_video->list_srt.clear();
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly)){
        QList<MyVideoWidget::SrtFile*> new_list;
        while(!file.atEnd()){
            file.readLine();
            QString data_time=file.readLine();
            QString data_text=file.readLine();
            file.readLine();
            QStringList data_start_time=data_time.split(" --> ").first().split(":");
            int h_start=data_start_time[0].toInt();
            int m_start=data_start_time[1].toInt();
            int s_start=data_start_time[2].split(",").first().toInt();
            int ms_start=data_start_time[2].split(",").last().toInt();
            QTime time_start(h_start,m_start,s_start,ms_start);
            // qDebug()<<"time_start"<<time_start;
            QStringList data_end_time=data_time.split(" --> ").last().split(":");
            int h_end=data_end_time[0].toInt();
            int m_end=data_end_time[1].toInt();
            int s_end=data_end_time[2].split(",").first().toInt();
            int ms_end=data_end_time[2].split(",").last().toInt();
            QTime time_end(h_end,m_end,s_end,ms_end);
            // qDebug()<<"time_end"<<time_end;
            MyVideoWidget::SrtFile* srt=new MyVideoWidget::SrtFile;
            srt->start=time_start;
            srt->end=time_end;
            srt->text=data_text;
            new_list.append(srt);
        }
        ui->widget_video->list_srt=std::move(new_list);
    }
}

void MainWindow::fillScreen()
{
    const bool isPlaying=player->isPlaying();
    if(isPlaying){
        emit ui->widget_video->w_box->btn_play->clicked();
    }
    player->setVideoSink(nullptr);
    ui->stackedWidget->setCurrentIndex(1);
    // QTimer::singleShot(50,this,[=](){

    // });
    player->setVideoOutput(ui->widget_video_2);
    if(isPlaying){
        player->play();
    }
    ui->head_widget->hide();
    this->move(0,0);
    this->resize(this->screen()->size());
    sendMsg("双击退出全屏");
}

void MainWindow::escScreen()
{
    const bool isPlaying=player->isPlaying();
    if(isPlaying){
        emit ui->widget_video->w_box->btn_play->clicked();
    }
    player->setVideoOutput(nullptr);
    ui->stackedWidget->setCurrentIndex(0);
    player->setVideoSink(sink);
    if(isPlaying){
        player->play();
    }
    ui->head_widget->show();
    this->resize(this->current_size);
    this->move(this->current_pos);

}

void MainWindow::loadHistoryFromfile()
{
    QDir dir=dir_save_history;
    QStringList files = dir.entryList(QDir::Files);
    for(QString& filepath:files){
        if(QFileInfo(filepath).suffix()!="json"){
            continue;
        }
        QFile file(dir.filePath(filepath));
        // qDebug()<<"dir.filePath(filepath)"<<dir.filePath(filepath);
        if(file.open(QIODevice::ReadOnly)){
            QByteArray data=file.readAll();
            QJsonDocument doc(QJsonDocument::fromJson(data));
            if(doc.isNull()){
                qDebug()<<"doc is null";
                continue;
            }
            if(doc.isArray()){
                qDebug()<<"doc is a array";
            }else if(doc.isObject()){
                QJsonObject oj=doc.object();

                QString name=oj["name"].toString();
                QString source=oj["source"].toString();
                QString isMusic=oj["isMusic"].toString();
                this->list_history.append(source);

                VideoButton* btn=new VideoButton(ui->scrollAreaWidgetContents_history);
                btn->name=name;
                btn->loadSource(source);
                if(isMusic=="true"){
                    btn->setMusic(true);
                }
                // btn->stamp=QDateTime(oj["timestamp"].toString());
                btn->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
                btn->setFixedHeight(this->height()/6);
                btn->setStyleSheet("QPushButton{"
                                   "text-align:left;"
                                   "font-size:15px;"
                                   "}");
                QList<QLayoutItem*> list_todelete_history;
                for(int i=0;i<ui->layout_scroll_history->count();i++){
                    QLayoutItem* item_todelete=ui->layout_scroll_history->itemAt(i);
                    QSpacerItem* spacer=dynamic_cast<QSpacerItem*>(item_todelete);
                    if(spacer){
                        list_todelete_history.append(spacer);
                    }
                }
                for(QLayoutItem* item:list_todelete_history){
                    ui->layout_scroll_history->removeItem(item);
                    delete item;
                }
                ui->layout_scroll_history->addWidget(btn);
                ui->layout_scroll_history->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Ignored,QSizePolicy::Expanding));
                connect(btn,&VideoButton::clicked,this,[=](){
                    // setIndex(list_media.indexOf(source));

                    this->updateSource(source);

                    // player->setVideoOutput(ui->widget_video->video_widget);
                    player->play();
                });
                connect(btn,&VideoButton::willDelete,this,[=](){
                    btn->deleteLater();
                });
                connect(btn,&VideoButton::movePosition,this,[=](bool up){
                    QWidget* parent=dynamic_cast<QWidget*>(btn->parent());
                    if(parent){
                        bool ishistory=false;
                        if(parent==ui->scrollAreaWidgetContents_history){
                            ishistory=true;
                        }
                        QVBoxLayout* layout=ishistory?ui->layout_scroll_history:ui->layout_scroll;
                        if(layout){
                            int index=layout->indexOf(btn);
                            layout->removeWidget(btn);
                            if(up){
                                layout->insertWidget(index-1,btn);
                            }else{
                                layout->insertWidget(index+1,btn);
                            }
                            update();
                        }else{
                            qDebug()<<"layout类型转换失败";
                        }
                    }else{
                        qDebug()<<"parent类型转换失败";
                    }
                });

                }else{
                    qDebug()<<"is not a object";
                }
        }else{
            QMessageBox::warning(this,"警告","读取本地历史记录文件失败");
            return;
        }
    }

}

void MainWindow::updateSource(QString filename)
{
    QFileInfo fileinfo=QFileInfo(filename);
    if(fileinfo.exists()){
        QString suffix=fileinfo.suffix();
        const static QString suffix_video="mp4 avi mkv";
        const static QString suffix_music="mp3 wav ogg";
        if(suffix_video.contains(suffix)){
            qDebug()<<"视频文件";
            switchVideoStack(ui->page_video);
        }else if(suffix_music.contains(suffix)){
            qDebug()<<"音频文件";
            switchVideoStack(ui->page_gif);
        }else{
            QMessageBox::information(this,"提示",QString("不支持%1格式！").arg(suffix));
            return;
        }

        player->setSource(QUrl::fromLocalFile(filename));
        ui->groupBox->setTitle(filename.split("/").last().split(".").first());
        ui->widget_video->setFocus();
        ui->widget_video->repaint();



        for(QObject*& oj : ui->scrollAreaWidgetContents_history->children()+ui->scrollAreaWidgetContents_list->children()){//其他的预览窗口停止播放gif
            VideoButton* btn=dynamic_cast<VideoButton*>(oj);
            if(btn){
                if(btn->getSource()!=filename){
                    btn->stopGif();
                }else{
                    btn->playGif();
                }
            }
        }
        for(QObject* oj : ui->scrollAreaWidgetContents_history->children()){//当点击的某条视频存在历史观看记录，把这条历史记录移到最前面
            VideoButton* btn=dynamic_cast<VideoButton*>(oj);
            // qDebug()<<"child";
            if(btn){
                if(btn->getSource()!=filename){
                    btn->stopGif();
                }else{
                    // qDebug()<<"source";
                    ui->layout_scroll_history->removeWidget(btn);
                    ui->layout_scroll_history->insertWidget(0,btn);
                }
            }
        }


        // QAudioFormat format;
        // format.setSampleRate(16000);    // Whisper标准输入采样率
        // format.setChannelCount(1);      // 单声道
        // format.setSampleFormat(QAudioFormat::Int16); // 16-bit PCM

        // // 创建音频捕获设备（从默认音频输出捕获）

        // QAudioDevice output=QMediaDevices::defaultAudioOutput();//获取当前默认的播放设备（耳机，扬声器等）
        // audio_source = new QAudioSource(output,format,this);

        // // 启动捕获，获取IO设备
        // audioIO = audio_source->start();

        // // 连接音频数据到达信号
        // connect(audioIO, &QIODevice::readyRead, this,[=]()mutable{
        //     QByteArray data = audioIO->readAll(); // 读取当前可用数据
        //     ringbuffer->write(data); // 写入环形缓冲区
        // });

        qDebug()<<"current filename"<<filename;
    }else{
        QMessageBox::information(this,"提示","该文件已被删除或移动");
    }

}

void MainWindow::loadNextVideo()
{
    // mutex_load_video.lock();
    static int video_counter=0;
    if(queue_load_video.isEmpty()){
        video_counter=0;//队列库存清完了则全部计数清零，等待新的待处理文件入队
        loadingVideo=false;
        return;
    }
    VideoButton* btn=queue_load_video.first();
    QWidget* parent=dynamic_cast<QWidget*>(btn->parent());
    if(!parent){
        qDebug()<<"类型转换失败";
        return;
    }
    mutex_max_num_load_video_once.lock();
    int max_num=this->max_num_load_video_once;
    mutex_max_num_load_video_once.unlock();
    mutex_interval_load_video.lock();
    int interval=this->interval_load_video;
    mutex_interval_load_video.unlock();

    qDebug()<<"load common"<<video_counter;
    {
        video_counter++;//每处理一个视频就计数一次
        queue_load_video.removeFirst();//处理了就移除
        btn->playSource();
        // QApplication::processEvents();
    }
    if(video_counter>=max_num){
        qDebug()<<"max";
        video_counter-=max_num;//如果超出了允许的一次性最大的加载视频数量，则计数减一轮，增加继续加载的按钮等待用户指令
        QVBoxLayout* layout=ui->layout_scroll;
        QPushButton* btn_continue=new QPushButton(parent);
        btn_continue->setText(QString("继续加载%1个视频").arg(max_num));
        btn_continue->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
        int index=layout->indexOf(btn);
        layout->insertWidget(index+1,btn_continue);
        btn_continue->show();
        connect(btn_continue,&QPushButton::clicked,this,[=](){
            QTimer::singleShot(interval,this,[=](){
                loadingVideo=true;
                loadNextVideo();
            });
            btn_continue->deleteLater();
        });
    }else{
        QTimer::singleShot(interval,this,[=](){
            loadingVideo=true;
            loadNextVideo();
        });
    }
    // mutex_load_video.unlock();
}

void MainWindow::loadNextHistoryVideo()
{
    // mutex_load_video.lock();
    static int video_counter_history=0;
    if(queue_load_video_history.isEmpty()){
        video_counter_history=0;//队列库存清完了则全部计数清零，等待新的待处理文件入队
        loadingVideo=false;
        return;
    }
    VideoButton* btn=queue_load_video_history.first();
    QWidget* parent=dynamic_cast<QWidget*>(btn->parent());
    if(!parent){
        qDebug()<<"类型转换失败";
        return;
    }
    if(btn->isLoaded()){
        qDebug()<<"已加载有封面";
        return;
    }
    mutex_max_num_load_video_once.lock();
    int max_num=this->max_num_load_video_once;
    mutex_max_num_load_video_once.unlock();
    mutex_interval_load_video.lock();
    int interval=this->interval_load_video;
    mutex_interval_load_video.unlock();

    qDebug()<<"load video_counter_history"<<video_counter_history;
    {
        video_counter_history++;//每处理一个视频就计数一次
        queue_load_video_history.removeFirst();//处理了就移除
        btn->playSource();
        // QApplication::processEvents();
    }
    if(video_counter_history>=max_num){
        qDebug()<<"max";
        video_counter_history-=max_num;//如果超出了允许的一次性最大的加载视频数量，则计数减一轮，增加继续加载的按钮等待用户指令
        QVBoxLayout* layout=ui->layout_scroll_history;
        QPushButton* btn_continue=new QPushButton(parent);
        btn_continue->setText(QString("继续加载%1个视频").arg(max_num));
        btn_continue->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
        int index=layout->indexOf(btn);
        layout->insertWidget(index+1,btn_continue);
        btn_continue->show();
        connect(btn_continue,&QPushButton::clicked,this,[=](){

            QTimer::singleShot(interval,this,[=](){
                loadingVideo=true;
                loadNextHistoryVideo();
            });
            btn_continue->deleteLater();
        });
    }else{
        QTimer::singleShot(interval,this,[=](){
            loadingVideo=true;
            loadNextHistoryVideo();
        });
    }
    // mutex_load_video.unlock();
}

void MainWindow::deleteFilesFromPath(const QString &path)
{
    QDir dir(path);
    if(!dir.exists()){
        qDebug()<<"目录不存在";
        return;
    }
    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    QFileInfoList list=dir.entryInfoList();
    for(QFileInfo& entry:list){
        if(entry.isDir()){
            deleteFilesFromPath(entry.absoluteFilePath());
            dir.rmdir(entry.fileName());
        }else{
            dir.remove(entry.fileName());
        }
    }

}

void MainWindow::switchVideoStack(QWidget *page)
{
    if(page==ui->page_video){
        ui->widget_music->stopGif();
        ui->stack_video->setCurrentWidget(page);
        ui->widget_video->setFocus();
        ui->widget_video->update();
    }else if(page==ui->page_gif){
        if(player->isPlaying())
            player->stop();
        ui->stack_video->setCurrentWidget(page);
        // qDebug()<<"label size"<<ui->label_gif->size();
        ui->widget_music->playGif();
    }else{
        qDebug()<<"video stack 没有这个页面";
    }
}

void MainWindow::loadUserInfo()
{
    QDir dir=QDir::currentPath()+"/data";
    QFile file(dir.filePath("user.json"));
    if(file.open(QIODevice::ReadOnly)){
        QJsonDocument document(QJsonDocument::fromJson(file.readAll()));
        if(!document.isEmpty()){
            QJsonObject oj=document.object();
            if(!oj.isEmpty()){
                QJsonValue value;
                value=oj["dir_save_history"];
                if(!value.isNull()){
                    dir_save_history=value.toString();
                    ui->set_widget->edit_save_file->setText(dir_save_history);
                }
                value=oj["max_history_num"];
                if(!value.isNull()){
                    max_history_num=value.toInt();
                }
                value=oj["interval_load_video"];
                if(!value.isNull()){
                    interval_load_video=value.toInt();
                }
                value=oj["min_lumilance"];
                if(!value.isNull()){
                    min_lumilance=value.toDouble();
                }
                value=oj["isWhite"];
                if(!value.isNull()){
                    isWhite=value.toBool();
                    if(isWhite){
                        ui->set_widget->combo_theme->setCurrentText("浅色");
                    }else{
                        ui->set_widget->combo_theme->setCurrentText("深色");
                    }
                }
                value=oj["background_path"];
                if(!value.isNull()){
                    changeBackgroundkPixmap(value.toString());
                }
                value=oj["font_family"];
                if(!value.isNull()){
                    changeFont(value.toString());
                }
                value=oj["forward_sec"];
                if(!value.isNull()){
                    forward_sec=value.toInt();
                }
                value=oj["backward_sec"];
                if(!value.isNull()){
                    backward_sec=value.toInt();
                }
                value=oj["user_lists"];
                if(value.isArray()){
                    QJsonArray array_file=value.toArray();//文件列表
                    for(const QJsonValue &array_value : std::as_const(array_file)){
                        if(array_value.isObject()){
                            QJsonObject oj_file=array_value.toObject();//具体的文件对象
                            QString file_name=oj_file["file_name"].toString();
                            QList<QPair<QString,QString>> file_videos;
                            if(oj_file["videos"].isArray()){
                                QJsonArray array_video=oj_file["videos"].toArray();//文件的子视频列表
                                for(const QJsonValue &video_val : std::as_const(array_video)){
                                    if(array_value.isObject()){
                                        QJsonObject oj_video=array_value.toObject();//文件的具体子视频
                                        file_videos.append(qMakePair(oj_video["title"].toString(),oj_video["path"].toString()));
                                    }
                                }
                            }
                            mutex_user_list.lock();
                            user_lists.append(qMakePair(file_name,file_videos));
                            mutex_user_list.unlock();
                        }
                    }
                }
            }
        }
    }else{
        qDebug()<<"用户信息文件打开失败";
    }


}

void MainWindow::saveUserInfo()
{
    QJsonObject oj;
    oj["dir_save_history"]=dir_save_history;
    oj["max_history_num"]=max_history_num;
    oj["max_num_load_video_once"]=max_num_load_video_once;
    oj["interval_load_video"]=interval_load_video;
    oj["min_lumilance"]=min_lumilance;
    oj["isWhite"]=isWhite;
    oj["background_path"]=background_path;
    oj["font_family"]=font_family;
    oj["forward_sec"]=forward_sec;
    oj["backward_sec"]=backward_sec;

    QJsonArray userListJsonArray;

    mutex_user_list.lock();
    QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists_copy=user_lists;
    mutex_user_list.unlock();
    for (const auto& userPair:user_lists_copy) {
        QJsonObject fileJson;
        fileJson["file_name"] = userPair.first;

        QJsonArray videoListJsonArray;
        for (const auto& videoPair : userPair.second) {
            QJsonObject videoJson;
            videoJson["title"] = videoPair.first;
            videoJson["path"] = videoPair.second;
            videoListJsonArray.append(videoJson);
        }

        fileJson["videos"] = videoListJsonArray;
        userListJsonArray.append(fileJson);
    }

    oj["user_lists"] = userListJsonArray;

    QJsonDocument document(oj);
    QDir dir=QDir::currentPath()+"/data";
    if(!dir.exists()){
        if(!dir.mkpath(dir.path())){
            QMessageBox::information(this,"提示","无法创建用户目录");
        }
    }
    QFile file(dir.filePath("user.json"));
    if(file.open(QIODevice::WriteOnly)){
        if(document.isEmpty()){
            qDebug()<<"QJsonDocument 为空";
        }else{
            file.write(document.toJson());
            file.close();
        }
    }else{
        qDebug()<<"文件打开失败";
    }
}

void MainWindow::changeBackgroundkPixmap(const QString& pxp)
{
    ui->set_widget->edit_pxp_path->setText(pxp);
    this->background_path=pxp;
    pxp_background=QPixmap(this->background_path).scaled(this->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    update();
}

void MainWindow::changeFont(const QString &font)
{
    QApplication::setFont(QFont(font));
    this->font_family=font;
    QString style=QString("*{font-size:15px;font-family:%1 !important;}").arg(QFont(font).family());
    // this->setStyleSheet(style);
    // ui->widget_2->setStyleSheet(style);
    ui->edit_seek->setStyleSheet(style);
    ui->scrollArea_list->setStyleSheet(style);

    ui->widget_video->w_box->combo_mod->w_combo->setStyleSheet(style);
    ui->widget_video->w_box->combo_speed->setStyleSheet(style);
    ui->widget_video->w_box->combo_speed->w_combo->setStyleSheet(style);

    style=QString("*{font-size:15px;font-family:%1 !important;color:black;}").arg(QFont(font).family());
    ui->set_widget->setStyleSheet(style);
    ui->scrollArea_ai->setStyleSheet(style);
}

// QString MainWindow::getAccessToken(const QString &url,const QString &apiKey, const QString &secretKey)
// {
//     QEventLoop loop;
//     QString request_url = url.arg(apiKey, secretKey);

//     QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(request_url)));
//     connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);

//     loop.exec();

//     QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//     return doc.object()["access_token"].toString();
// }

QString MainWindow::generateAuthorization(const QString &secretId, const QString &secretKey, const QString &httpMethod, const QString &key, const QString &bucket, const QString &region) {
    // 1. 时间戳（假设系统时间正确）
    QDateTime currentTime = QDateTime::currentDateTimeUtc();
    QString date = currentTime.toString("yyyyMMddTHHmmssZ");
    QString shortDate = currentTime.toString("yyyyMMdd");

    // 2. 构造 CanonicalRequest（严格遵循腾讯云格式）
    QString canonicalUri = "/" + key;  // 对象键需以斜杠开头
    QString canonicalQuery = "";       // 无查询参数时保留空行
    QString canonicalHeaders = QString("host:%1.cos.%2.myqcloud.com\n").arg(bucket, region);
    QString signedHeaders = "host";    // 明确列出已签名的头

    QString canonicalRequest = QString("%1\n%2\n%3\n%4\n%5\nUNSIGNED-PAYLOAD")
                                   .arg(httpMethod.toLower(),  // HTTP方法小写
                                        canonicalUri,
                                        canonicalQuery,
                                        canonicalHeaders,
                                        signedHeaders);

    // 3. 计算 HashedCanonicalRequest
    QByteArray hashedCanonicalRequest = QCryptographicHash::hash(canonicalRequest.toUtf8(), QCryptographicHash::Sha256).toHex();

    // 4. 构造 StringToSign（修正CredentialScope顺序）
    QString credentialScope = QString("%1/%2/cos/request").arg(shortDate, region);
    QString stringToSign = QString("TC3-HMAC-SHA256\n%1\n%2\n%3")
                               .arg(date, credentialScope, QString(hashedCanonicalRequest));

    // 5. 计算签名密钥（严格分层计算）
    QByteArray kSigning, kService, kDate;
    QByteArray kSecret = ("TC3" + secretKey).toUtf8();
    kDate = QMessageAuthenticationCode::hash(shortDate.toUtf8(), kSecret, QCryptographicHash::Sha256);
    kService = QMessageAuthenticationCode::hash("cos", kDate, QCryptographicHash::Sha256);
    kSigning = QMessageAuthenticationCode::hash("request", kService, QCryptographicHash::Sha256);

    // 6. 计算最终签名
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign.toUtf8(), kSigning, QCryptographicHash::Sha256).toHex();

    // 7. 构造 Authorization 头
    QString authorization = QString("TC3-HMAC-SHA256 Credential=%1/%2, SignedHeaders=%3, Signature=%4")
                                .arg(secretId, credentialScope, signedHeaders, QString(signature));

    return authorization;
}

void MainWindow::uploadFileToCOS(const QString &filePath, const QString &bucket, const QString &region, const QString &secretId, const QString &secretKey) {
    QString objectPath = "doc/" + QFileInfo(filePath).fileName(); // 假设你上传到 doc 目录
    QString urlString = QString("https://%1.cos.%2.myqcloud.com/%3").arg(bucket, region, objectPath);
    QUrl url(urlString);

    // 设置请求头
    QString authorization = generateAuthorization(secretId, secretKey, "PUT", QFileInfo(filePath).fileName(), bucket, region);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", authorization.toUtf8());

    // 打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << filePath;
        return;
    }

    // 创建网络访问管理器
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.put(request, file.readAll());
    file.close();

    // 处理响应
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "File uploaded successfully.";
        } else {
            qDebug() << "Upload failed:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
