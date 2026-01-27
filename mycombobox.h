#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QPushButton>
#include <QToolButton>
#include <QWidget>

class MyComboBox : public QWidget
{
    Q_OBJECT
public:
    explicit MyComboBox(QWidget *parent = nullptr);
    ~MyComboBox();
    void paintEvent(QPaintEvent* ev)override;
    void mousePressEvent(QMouseEvent* ev)override;
    void focusOutEvent(QFocusEvent* ev)override;
    void resizeEvent(QResizeEvent* ev)override;

    QList<QWidget*> getWidgets()const{return list_widget;};
    void addWidget(QWidget* w_new);
    QToolButton* getMainButton(){return btn_main;};
    void setMainButton(QToolButton* btn_new);
    void removeWidget(int index);
    void setDirection(bool new_up_direction);
    void setIcon(QIcon icon);
    void setIconSize(QSize size);

    QWidget* w_combo;
    QToolButton* btn_main;
public slots:
    void show_combo();
    void close_combo();
signals:

private:
    QList<QWidget*> list_widget;


    bool up_direction=true;
    bool showing_combo=false;
};

#endif // MYCOMBOBOX_H
