#ifndef CONTEXTBUTTON_H
#define CONTEXTBUTTON_H

#include <QPushButton>

class ContextButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ContextButton(QString text,QWidget *parent = nullptr);
    void contextMenuEvent(QContextMenuEvent* ev)override;

    static bool hasSameName(const QString &list_name, const QString &video_name=NULL);
    // void addContextButton(ContextButton* new_btn);

    bool video_button=false;
    QList<QPair<QString,QString>> list;
    QString parentname;
signals:
    void addToCurrentList();
    void playAll();
    void willDelete();
    void movePosition(bool up);
private:

};

#endif // CONTEXTBUTTON_H
