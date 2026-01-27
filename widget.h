#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void keyPressEvent(QKeyEvent* ev)override;
    void dragEnterEvent(QDragEnterEvent* ev)override;
    void dropEvent(QDropEvent* ev)override;
    bool eventFilter(QObject *obj, QEvent *ev)override;

    void setIndex(int new_index);
    void sendMsg(QString msg);
    void addFile(QString filename);
private:
    Ui::Widget *ui;
    QMediaPlayer* player;
    QStringList list_media;
    int index_list_media=-1;//当前所播放的视频在list中的索引
    int play_mod=1;//默认顺序播放
};
#endif // WIDGET_H
