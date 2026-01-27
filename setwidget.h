#ifndef SETWIDGET_H
#define SETWIDGET_H

#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class SetWidget;
}
QT_END_NAMESPACE

class SetWidget : public QWidget
{
    Q_OBJECT

public:
    SetWidget(QWidget *parent = nullptr);
    ~SetWidget();
    Ui::SetWidget *ui;

    QPushButton* btn_clear_user_data;
    QPushButton* btn_open_file;
    QPushButton* btn_choose_font;
    QPushButton* btn_load_all;
    QPushButton* btn_open_file_pxp;
    QPushButton* btn_default;
    QPushButton* btn_save_all;
    QComboBox* combo_theme;
    QSpinBox* spin_max_history_num;
    QSpinBox* spin_max_num_load_video;
    QSpinBox* spin_lumilance;
    QSpinBox* spin_interval_load_video;
    QSpinBox* spin_forward;
    QSpinBox* spin_backward;
    QLineEdit* edit_save_file;
    QLineEdit* edit_pxp_path;
    QComboBox* combo_modify_pause;
    QComboBox* combo_pause;
    QComboBox* combo_modify_forward;
    QComboBox* combo_forward;
    QComboBox* combo_modify_backward;
    QComboBox* combo_backward;
    QComboBox* combo_modify_fill;
    QComboBox* combo_fill;

private:

};
#endif // SETWIDGET_H
