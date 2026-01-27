#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // qputenv("QSG_RHI_BACKEND", "software");
    QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    MainWindow w;
    // qDebug()<<QVideo
    w.show();
    return a.exec();
}
