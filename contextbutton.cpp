#include "contextbutton.h"

#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMutex>

extern QMutex mutex_user_list;
extern QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists;

ContextButton::ContextButton(QString text, QWidget *parent)
    : QPushButton{parent}
{
    this->setText(text);
}

void ContextButton::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu* menu=new QMenu(this);


    QAction* act_play=new QAction(video_button?"播放":"播放全部",menu);
    QAction* act_rename=new QAction("重命名",menu);
    QAction* act_delete=new QAction("删除",menu);
    QMenu* menu_move=new QMenu("移动",menu);
    QAction* act_up=new QAction("上移",menu);
    QAction* act_down=new QAction("下移",menu);

    menu->addAction(act_play);
    if(video_button){
        QAction* act_add_to_current=new QAction("添加至当前列表",menu);
        connect(act_add_to_current,&QAction::triggered,this,[=](){
            emit addToCurrentList();
        });
        menu->addAction(act_add_to_current);
    }
    menu->addSeparator();
    menu->addAction(act_rename);
    menu->addAction(act_delete);
    menu->addMenu(menu_move);
    menu_move->addAction(act_up);
    menu_move->addAction(act_down);



    connect(act_play,&QAction::triggered,this,[=](){
        if(video_button){
            emit this->clicked();
        }else{
            emit playAll();
        }
    });
    connect(act_rename,&QAction::triggered,this,[=](){
        QDialog* dlg=new QDialog(this);
        QHBoxLayout* hlayout=new QHBoxLayout(dlg);
        QLineEdit* edit=new QLineEdit(this->text(),dlg);
        QPushButton* btn=new QPushButton("确认",this);

        dlg->setWindowTitle("重命名:");

        hlayout->addWidget(edit,9);
        hlayout->addWidget(btn,1);

        connect(btn,&QPushButton::clicked,this,[=](){
            QString name_new=edit->text();
            if(!name_new.isEmpty()){
                if(video_button){
                    if(hasSameName(parentname,name_new)){
                        QMessageBox::information(this,"提示","名称不能重复!");
                    }else{
                        QString name_old=this->text();
                        QString parentname=this->parentname;
                        for(QPair<QString,QList<QPair<QString,QString>>>& pair:user_lists){
                            if(pair.first==parentname){
                                for(QPair<QString,QString>& pair_2:pair.second){
                                    if(pair_2.first==name_old){
                                        pair_2.first=name_new;
                                    }
                                }
                            }
                        }
                        this->setText(name_new);
                        dlg->close();
                    }
                }else{
                    if(hasSameName(name_new)){
                        QMessageBox::information(this,"提示","名称不能重复!");
                    }else{
                        QString name_old=this->text();
                        for(QPair<QString,QList<QPair<QString,QString>>>& pair:user_lists){
                            if(pair.first==name_old){
                                pair.first=name_new;
                            }
                        }
                        this->setText(name_new);
                        dlg->close();
                    }
                }
            }
        });

        dlg->exec();
        dlg->deleteLater();
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

bool ContextButton::hasSameName(const QString& list_name,const QString& video_name)//子视频名称为空则是判断文件夹名有无重复
{
    mutex_user_list.lock();
    QVector<QPair< QString,QList< QPair<QString,QString> > >> user_lists_copy=user_lists;
    mutex_user_list.unlock();
    for(QPair< QString,QList< QPair<QString,QString> > >& pair:user_lists_copy){
        if(pair.first==list_name){
            if(video_name.isEmpty()){
                return true;
            }else{
                for(QPair<QString,QString>& pair_2:pair.second){
                    if(pair_2.first==video_name){
                        return true;
                    }
                }
                return false;
            }

        }
    }
    return false;
}
