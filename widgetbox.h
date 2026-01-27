#ifndef WIDGETBOX_H
#define WIDGETBOX_H

#include "mycombobox.h"

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>

class WidgetBox : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetBox(QWidget *parent = nullptr);
    QSize sizeHint()const override;

    void changeTheme(bool isWhite);

    QToolButton* btn_last;
    QToolButton* btn_play;
    QToolButton* btn_next;
    QLabel*      label_progress;
    MyComboBox*  combo_ratio;
    MyComboBox*  combo_speed;
    MyComboBox*  combo_mod;
    MyComboBox*  combo_volume;
    QToolButton* btn_set;
    QToolButton* btn_full;
signals:
private:
};

#endif // WIDGETBOX_H
