#include "mycombobox.h"

#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

MyComboBox::MyComboBox(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout* layout=new QVBoxLayout(this);
    this->btn_main=new QToolButton(this);

    this->layout()->addWidget(btn_main);
    this->setStyleSheet("QToolButton{"
                            "font-size:20px;"
                            "font-family:微软雅黑;"
                            "}");
    connect(btn_main,&QPushButton::clicked,this,[=](){
        qDebug()<<"showing_combo"<<showing_combo;
        show_combo();
    });

    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    this->setLayout(layout);
    setMainButton(btn_main);

    this->w_combo=new QWidget;
    w_combo->setFocusPolicy(Qt::StrongFocus);
    w_combo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    QVBoxLayout* layout_combo=new QVBoxLayout(w_combo);
    layout_combo->setSpacing(0);
    layout_combo->setContentsMargins(0,0,0,0);
    w_combo->setWindowFlags(Qt::FramelessWindowHint|Qt::Popup);
    w_combo->setLayout(layout_combo);
    w_combo->setVisible(false);
    // // 使用QGraphicsDropShadowEffect
    // QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(w_combo);
    // shadow->setBlurRadius(10);
    // shadow->setColor(QColor(0, 0, 0, 60));
    // shadow->setOffset(0, 2);
    // w_combo->setGraphicsEffect(shadow);
}

MyComboBox::~MyComboBox()
{
    delete w_combo;
    //qDebug()<<"MyComboBox析构";
}

void MyComboBox::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);
}

void MyComboBox::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button()==Qt::LeftButton){

    }
}

void MyComboBox::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);
    close_combo();
    //qDebug()<<"focusOutEvent";
    QWidget::focusOutEvent(ev);
}

void MyComboBox::resizeEvent(QResizeEvent *ev)
{
    this->w_combo->setFixedWidth(ev->size().width());
}

void MyComboBox::addWidget(QWidget *w_new)
{
    // qDebug()<<"add widget";
    w_new->setParent(this);
    w_new->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    list_widget.append(w_new);
    w_combo->layout()->addWidget(w_new);
}

void MyComboBox::setMainButton(QToolButton *btn_new)
{
    if(btn_main!=btn_new){
        this->btn_main->deleteLater();
        this->btn_main=btn_new;
        btn_new->setParent(this);
        this->layout()->addWidget(btn_main);
        connect(btn_main,&QPushButton::clicked,this,[=](){
            qDebug()<<"showing_combo"<<showing_combo;
            show_combo();
            // if(showing_combo){//因为flag的popup属性，窗口关闭不受控制，showing_combo在popup自动管理关闭时没有更改
            //     close_combo();
            // }else{
            //     show_combo();
            // }
        });
    }
}

void MyComboBox::removeWidget(int index)
{
    if(index>0 && index<list_widget.size()){
        QWidget* w_todelete=list_widget.at(index);
        list_widget.removeOne(w_todelete);
        w_combo->layout()->removeWidget(w_todelete);
        w_todelete->deleteLater();
    }else{
        qDebug()<<"removeWidget操作越界";
    }
}

void MyComboBox::setDirection(bool new_up_direction)
{
    if(up_direction!=new_up_direction){
        up_direction=new_up_direction;
        update();
    }
}

void MyComboBox::setIcon(QIcon icon)
{
    this->btn_main->setIcon(icon);
    update();
}

void MyComboBox::setIconSize(QSize size)
{
    this->btn_main->setIconSize(size);
}

void MyComboBox::show_combo()
{
    this->showing_combo=true;
    w_combo->show();
    //qDebug()<<"w_combo"<<w_combo->geometry();
    if(up_direction){
        QPoint globalpos=this->mapToGlobal(QPoint(0,0));
        globalpos.setY(globalpos.y()-w_combo->height());
        w_combo->move(globalpos);
        //qDebug()<<"w_combo->height()"<<w_combo->height();
    }
}

void MyComboBox::close_combo()
{
    this->showing_combo=false;
    w_combo->close();
}
