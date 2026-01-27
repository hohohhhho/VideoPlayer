#ifndef MYHINTPUSHBUTTON_H
#define MYHINTPUSHBUTTON_H

#include <QPushButton>

class MyHintPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyHintPushButton(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent* ev)override;
    QSize sizeHint()const override;
signals:
};

#endif // MYHINTPUSHBUTTON_H
