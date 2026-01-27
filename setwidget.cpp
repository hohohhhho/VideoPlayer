#include "setwidget.h"
#include "ui_setwidget.h"

SetWidget::SetWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetWidget)
{
    ui->setupUi(this);
    ui->widget_save->setVisible(false);
    ui->widget_load_video->setVisible(false);
    ui->widget_theme->setVisible(false);
    ui->widget_hotkey->setVisible(false);

    this->btn_clear_user_data=ui->btn_clear_user_data;
    this->btn_open_file=ui->btn_open_file;
    this->btn_choose_font=ui->btn_choose_font;
    this->btn_load_all=ui->btn_load_all;
    this->btn_open_file_pxp=ui->btn_open_file_pxp;
    this->btn_default=ui->btn_default;
    this->btn_save_all=ui->btn_save_all;
    this->combo_theme=ui->combo_theme;
    this->spin_max_history_num=ui->spin_max_history_num;
    this->spin_max_num_load_video=ui->spin_max_num_load_video;
    this->spin_interval_load_video=ui->spin_interval_load_video;
    this->spin_lumilance=ui->spin_lumilance;
    this->spin_backward=ui->spin_backward;
    this->spin_forward=ui->spin_forward;
    this->edit_save_file=ui->edit_save_path;
    this->edit_pxp_path=ui->edit_pxp_path;
    this->combo_modify_pause=ui->combo_modify_pause;
    this->combo_pause=ui->combo_pause;
    this->combo_modify_forward=ui->combo_modify_forward;
    this->combo_forward=ui->combo_forward;
    this->combo_modify_backward=ui->combo_modify_backward;
    this->combo_backward=ui->combo_backward;
    this->combo_modify_fill=ui->combo_modify_fill;
    this->combo_fill=ui->combo_fill;

    connect(ui->drawer_save,&QPushButton::clicked,this,[=](){
        if(ui->widget_save->isVisible()){
            ui->widget_save->setVisible(false);
        }else{
            ui->widget_save->setVisible(true);
            ui->widget_load_video->setVisible(false);
            ui->widget_theme->setVisible(false);
            ui->widget_hotkey->setVisible(false);
        }

    });
    connect(ui->drawer_load_video,&QPushButton::clicked,this,[=](){
        if(ui->widget_load_video->isVisible()){
            ui->widget_load_video->setVisible(false);
        }else{
            ui->widget_save->setVisible(false);
            ui->widget_load_video->setVisible(true);
            ui->widget_theme->setVisible(false);
            ui->widget_hotkey->setVisible(false);
        }
    });
    connect(ui->drawer_theme,&QPushButton::clicked,this,[=](){
        if(ui->widget_theme->isVisible()){
            ui->widget_theme->setVisible(false);
        }else{
            ui->widget_save->setVisible(false);
            ui->widget_load_video->setVisible(false);
            ui->widget_theme->setVisible(true);
            ui->widget_hotkey->setVisible(false);
        }
    });
    connect(ui->drawer_hotkey,&QPushButton::clicked,this,[=](){
        if(ui->widget_hotkey->isVisible()){
            ui->widget_hotkey->setVisible(false);
        }else{
            ui->widget_save->setVisible(false);
            ui->widget_load_video->setVisible(false);
            ui->widget_theme->setVisible(false);
            ui->widget_hotkey->setVisible(true);
        }
    });

}

SetWidget::~SetWidget()
{
    delete ui;
}
