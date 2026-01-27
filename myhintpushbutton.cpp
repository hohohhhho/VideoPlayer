#include "myhintpushbutton.h"

#include <QResizeEvent>

MyHintPushButton::MyHintPushButton(QWidget *parent)
    : QPushButton{parent}
{}

void MyHintPushButton::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    this->resize(this->height(),this->height());
    this->setIconSize(this->size()*0.9);
}

QSize MyHintPushButton::sizeHint() const
{
    return QSize(30,30);//疑问点，值为（100，40）时会把弹簧顶没
}
