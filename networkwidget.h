#ifndef NETWORKWIDGET_H
#define NETWORKWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui {
class NetworkWidget;
}
QT_END_NAMESPACE

class NetworkWidget : public QWidget
{
    Q_OBJECT

public:
    NetworkWidget(QWidget *parent = nullptr);
    ~NetworkWidget();

private:
    Ui::NetworkWidget *ui;
    QNetworkAccessManager* manager;
    QString search;

    QList<QPair<QString, QString> > extractVideoLinks(const QString &html);

    int website=1;//1为B站，2为QQ音乐
};
#endif // NETWORKWIDGET_H
