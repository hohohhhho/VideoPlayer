#ifndef USERLISTWIDGET_H
#define USERLISTWIDGET_H

#include <QWidget>
#include "contextbutton.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class UserListWidget;
}
QT_END_NAMESPACE

class UserListWidget : public QWidget
{
    Q_OBJECT

public:
    UserListWidget(QWidget *parent = nullptr);
    ~UserListWidget();

    void init();

signals:
    void playVideo(const QString& source);
    void addToCurrentList(const QString& source);
private:
    Ui::UserListWidget *ui;
    ContextButton* btn_current;

    ContextButton *createFileButton(const QString &title, bool add_to_list=true);
    ContextButton *createVideoButton(const QString &parentname,const QString &name,const QString &path,bool add_to_list=true);
    // bool hasSameName(const QString& list_name, const QString &video_name=NULL);
};
#endif // USERLISTWIDGET_H
