#include "userlistwidget.h"
#include "ui_userlistwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMutex>

extern QMutex mutex_user_list;
extern QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists;

UserListWidget::UserListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserListWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("我的音视频列表");
    this->setWindowIcon(QIcon(":/res/mod_list.png"));
    this->setWindowFlag(Qt::WindowStaysOnTopHint);

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->stackedWidget->setCurrentWidget(ui->page_main);

    this->btn_current=nullptr;

    connect(ui->btn_add_list,&QPushButton::clicked,this,[=](){
        QString file_name="未命名文件夹";
        while(ContextButton::hasSameName(file_name)){
            file_name.append("*");
        }
        createFileButton(file_name,true);
    });

    connect(ui->btn_add_video,&QPushButton::clicked,this,[=](){
        QStringList list_filename=QFileDialog::getOpenFileNames(this,"打开文件",QDir::currentPath(),"音视频文件(*.mp3 *.wav *.mp4 *.avi *.mkv);;所有文件(*)");
        // qDebug()<<"filename"<<filename;
        for(QString& filename:list_filename){
            if(!filename.isEmpty() && this->btn_current!=nullptr){
                ContextButton* temp_btn_current=this->btn_current;
                QString name=filename.split(".").first().split("/").last();
                QString parentname=temp_btn_current->text();
                while(ContextButton::hasSameName(parentname,name)){
                    name.append("*");
                }

                createVideoButton(parentname,name,filename,true);

            }
        }
    });

    connect(ui->btn_back,&QPushButton::clicked,this,[=](){
        for(int i=0;i<ui->layout_detail->count();i++){
            QWidget* item=ui->layout_detail->itemAt(i)->widget();
            ContextButton* btn=dynamic_cast<ContextButton*>(item);
            if(btn){
                btn->deleteLater();
            }
        }
        this->btn_current=nullptr;
        ui->stackedWidget->setCurrentWidget(ui->page_main);
    });

    connect(ui->edit_seek,&QLineEdit::returnPressed,this,[=](){
        emit ui->btn_seek->clicked();
    });

    connect(ui->btn_seek,&QPushButton::clicked,this,[=](){
        QString key=ui->edit_seek->text();
        if(!key.isEmpty()){
            for(QObject* oj:ui->widget->children()){
                ContextButton* btn=dynamic_cast<ContextButton*>(oj);
                if(btn){
                    for(auto& video:btn->list){
                        if(video.first.contains(key)){
                            emit btn->clicked();
                            return;
                        }
                    }
                }
            }
        }
    });

    connect(ui->btn_clear,&QPushButton::clicked,this,[=](){
        if(QMessageBox::StandardButton(QMessageBox::Yes)==
            QMessageBox::warning(this,"提示","确定要删除所有音视频列表吗?"
                                     ,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No))){
            mutex_user_list.lock();
            user_lists.clear();
            mutex_user_list.unlock();
        }
    });

    connect(ui->btn_refresh,&QPushButton::clicked,this,[=](){
        this->btn_current=nullptr;
        for(int i=0;i<ui->layout_detail->count();i++){
            QWidget* item=ui->layout_detail->itemAt(i)->widget();
            ContextButton* btn=dynamic_cast<ContextButton*>(item);
            if(btn){
                btn->deleteLater();
            }
        }
        for(QObject* oj:ui->widget->children()+ui->widget_detail->children()){
            ContextButton* btn=dynamic_cast<ContextButton*>(oj);
            if(btn){
                btn->deleteLater();
            }
        }
        init();
        ui->stackedWidget->setCurrentWidget(ui->page_main);
    });
}

UserListWidget::~UserListWidget()
{
    delete ui;
}

void UserListWidget::init()
{
    //QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists;//用户的视频列表
    mutex_user_list.lock();
    QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists_copy=user_lists;
    mutex_user_list.unlock();
    for(QPair< QString,QList< QPair<QString,QString>>>& pair1:user_lists_copy){
        ContextButton* file=createFileButton(pair1.first,false);
        for(QPair<QString,QString>& pair2:pair1.second){
            file->list.append(pair2);
        }

    }

}

ContextButton* UserListWidget::createFileButton(const QString &title,bool add_to_list)
{
    QList< QPair<QString,QString> > list;
    if(add_to_list){
        mutex_user_list.lock();
        user_lists.append(qMakePair(title,list));
        mutex_user_list.unlock();
    }

    ContextButton* btn=new ContextButton(title,this);
    connect(btn,&ContextButton::clicked,this,[=](){
        this->btn_current=btn;
        for(QPair<QString,QString>& pair:btn->list){
            ContextButton* temp_btn_current=this->btn_current;
            QString name=pair.first;
            QString filename=pair.second;
            QString parentname=temp_btn_current->text();

            createVideoButton(parentname,name,filename,false);
        }
        ui->label_detail->setText(btn->text()+":");
        ui->stackedWidget->setCurrentWidget(ui->page_detail);
    });
    connect(btn,&ContextButton::willDelete,this,[=](){
        if(this->btn_current==btn)
            this->btn_current=nullptr;
        mutex_user_list.lock();
        user_lists.removeOne(qMakePair(btn->text(),btn->list));
        mutex_user_list.unlock();
        btn->deleteLater();
    });

    QVBoxLayout* vlayout=dynamic_cast<QVBoxLayout*>(ui->widget->layout());
    if(vlayout){
        vlayout->insertWidget(0,btn);
    }

    return btn;
}

ContextButton *UserListWidget::createVideoButton(const QString &parentname, const QString &name, const QString &path, bool add_to_list)
{
    ContextButton* temp_btn_current=this->btn_current;
    ContextButton* video=new ContextButton(name,ui->widget_detail);
    video->setMaximumWidth(400);
    video->parentname=parentname;
    video->video_button=true;

    ui->layout_detail->insertWidget(0,video);
    connect(video,&ContextButton::clicked,this,[=](){
        emit playVideo(path);
    });
    connect(video,&ContextButton::willDelete,this,[=](){
        mutex_user_list.lock();
        // QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists_copy=user_lists;
        for(QPair< QString,QList< QPair<QString,QString> > >& pair:user_lists){
            if(pair.first==temp_btn_current->text()){
                pair.second.removeOne(qMakePair(name,path));
            }
        }
        mutex_user_list.unlock();
    });
    connect(video,&ContextButton::addToCurrentList,this,[=](){
        emit addToCurrentList(path);
    });
    if(add_to_list){
        temp_btn_current->list.append(qMakePair(name,path));
        mutex_user_list.lock();
        // QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists_copy=user_lists;
        for(QPair< QString,QList< QPair<QString,QString> > >& pair:user_lists){
            if(pair.first==temp_btn_current->text()){
                // qDebug()<<"apppend";
                pair.second.append(qMakePair(name,path));
            }
        }
        mutex_user_list.unlock();
    }
    return video;
}


