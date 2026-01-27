#ifndef HEADBUTTON_H
#define HEADBUTTON_H

#include <QPushButton>
#include <QWidget>

class HeadButton : public QWidget
{
    Q_OBJECT
public:
    explicit HeadButton(QWidget* parent);
    void paintEvent(QPaintEvent* ev)override;
    void resizeEvent(QResizeEvent* ev)override;
    void mousePressEvent(QMouseEvent* ev)override;
    void mouseMoveEvent(QMouseEvent* ev)override;
    void mouseReleaseEvent(QMouseEvent* ev)override;
    void leaveEvent(QEvent* ev)override;

    void changeTheme(bool theme_white);

    QColor color_background;
    QColor color_normal;
    QColor color_clicked;
    QColor color_hover;

signals:
    void clicked(int index_button);//从0开始
private:
    int x_start=0;
    int w_btn=0;
    int index_hovering=-1;
    int index_clicking=-1;
    bool fixed=false;
    bool theme_white=true;
};

#endif // HEADBUTTON_H
