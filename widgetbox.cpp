#include "widgetbox.h"

#include "mycombobox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

WidgetBox::WidgetBox(QWidget *parent)
    : QWidget{parent}
{
    //QVBoxLayout* layout_v=new QVBoxLayout(this);
    QHBoxLayout* layout_h=new QHBoxLayout(this);
    this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    this->setVisible(false);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);
    this->setStyleSheet("QLabel{"
                        "font-size:15px;"
                        "}");


    btn_last=new QToolButton(this);
    btn_play=new QToolButton(this);
    btn_next=new QToolButton(this);
    label_progress=new QLabel("00:00:00/00:00:00",this);
    combo_ratio=new MyComboBox(this);
    combo_mod=new MyComboBox(this);
    combo_speed=new MyComboBox(this);
    combo_volume=new MyComboBox(this);
    btn_set=new QToolButton(this);
    btn_full=new QToolButton(this);

    //layout_v->addWidget(bar);
    //layout_v->addLayout(layout_h);
    layout_h->addWidget(btn_last,1);
    layout_h->addWidget(btn_play,1);
    layout_h->addWidget(btn_next,1);
    layout_h->addWidget(label_progress,2);
    //layout_h->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed));
    layout_h->addWidget(combo_ratio,1);
    layout_h->addWidget(combo_mod,1);
    layout_h->addWidget(combo_speed,1);
    layout_h->addWidget(combo_volume,1);
    layout_h->addWidget(btn_set,1);
    layout_h->addWidget(btn_full,1);

    //layout_v->setContentsMargins(0,0,0,0);
    //layout_v->setSpacing(0);
    layout_h->setContentsMargins(0,0,0,0);
    layout_h->setSpacing(2);

    btn_last->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn_play->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn_next->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn_set->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn_full->setToolButtonStyle(Qt::ToolButtonIconOnly);

    label_progress->setAlignment(Qt::AlignCenter);
    btn_play->setText("播放");
    combo_speed->btn_main->setText("1.0");

    this         ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    btn_last      ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    btn_play     ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    btn_next      ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    label_progress->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_ratio   ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_mod   ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_speed   ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_volume  ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    btn_set       ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    btn_full      ->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);

    btn_last      ->setToolTip("上一个");
    btn_play      ->setToolTip("播放/暂停");
    btn_next      ->setToolTip("下一个");
    label_progress->setToolTip("进度");
    combo_ratio   ->setToolTip("暂未定");
    combo_mod     ->setToolTip("播放模式");
    combo_speed   ->setToolTip("播放速度");
    combo_volume  ->setToolTip("音量");
    btn_set       ->setToolTip("设置");
    btn_full      ->setToolTip("全屏");

    combo_ratio   ->btn_main->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_mod     ->btn_main->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_speed   ->btn_main->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    combo_volume  ->btn_main->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);

    this         ->setMinimumSize(0,0);
    btn_last      ->setMinimumSize(0,0);
    btn_play     ->setMinimumSize(0,0);
    btn_next      ->setMinimumSize(0,0);
    label_progress->setMinimumSize(0,0);
    combo_ratio   ->setMinimumSize(0,0);
    combo_mod   ->setMinimumSize(0,0);
    combo_speed   ->setMinimumSize(0,0);
    combo_volume  ->setMinimumSize(0,0);
    btn_set       ->setMinimumSize(0,0);
    btn_full      ->setMinimumSize(0,0);

    const int size=52;
    btn_last      ->resize(size,size);
    btn_play      ->resize(size,size);
    btn_next      ->resize(size,size);
    label_progress->resize(size,size);
    combo_ratio   ->resize(size,size);
    combo_mod     ->resize(size,size);
    combo_speed   ->resize(size,size);
    combo_volume  ->resize(size,size);
    btn_set       ->resize(size,size);
    btn_full      ->resize(size,size);

    combo_ratio   ->btn_main->resize(size,size);
    combo_mod     ->btn_main->resize(size,size);
    combo_speed   ->btn_main->resize(size,size);
    combo_volume  ->btn_main->resize(size,size);

    btn_last      ->setIconSize(QSize(size,size));
    btn_play      ->setIconSize(QSize(size,size));
    btn_next      ->setIconSize(QSize(size,size));
    Q_UNUSED(label_progress);
    combo_ratio   ->setIconSize(QSize(size,size));
    combo_mod     ->setIconSize(QSize(size,size));
    combo_speed   ->setIconSize(QSize(size,size));
    combo_volume  ->setIconSize(QSize(size,size));
    btn_set       ->setIconSize(QSize(size,size));
    btn_full      ->setIconSize(QSize(size,size));



    btn_last->setIcon(QIcon(":/res/last"));
    btn_play->setIcon(QIcon(":/res/start"));
    btn_next->setIcon(QIcon(":/res/next"));
    Q_UNUSED(label_progress);
    combo_ratio->setIcon(QIcon(":/res/last"));
    combo_mod->setIcon(QIcon(":/res/mod_sequent"));
    Q_UNUSED(combo_speed);
    combo_volume->setIcon(QIcon(":/res/volume"));
    btn_set->setIcon(QIcon(":/res/set"));
    btn_full->setIcon(QIcon(":/res/full"));



}

QSize WidgetBox::sizeHint() const
{
    return QSize(691,36);
}

void WidgetBox::changeTheme(bool isWhite)
{
    // btn_last      ->setStyleSheet("QToolButton{");
    // btn_play      ->setStyleSheet("QToolButton{");
    // btn_next      ->setStyleSheet("QToolButton{");
    // label_progress->setStyleSheet("QToolButton{");
    // combo_ratio   ->setStyleSheet("QToolButton{");
    // combo_mod     ->setStyleSheet("QToolButton{");
    // combo_speed   ->setStyleSheet("QToolButton{");
    // combo_volume  ->setStyleSheet("QToolButton{");
    // btn_set       ->setStyleSheet("QToolButton{");
    // btn_full      ->setStyleSheet("QToolButton{");
    if(isWhite){
        this->setStyleSheet("QToolButton{"
                            "background-color:white;"
                            "}");
    }else{
        this->setStyleSheet("QToolButton{"
                            "background-color:rgb(93,93,93);"
                            "}");
    }
}

