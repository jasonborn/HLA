#include "Mainwindow/mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qssF(":/qss/qss");
    if(qssF.open(QFile::ReadOnly))
    {
        QTextStream stream(&qssF);
        QString sss = stream.readAll();
        a.setStyleSheet(sss);
    }
    qssF.close();


    MainWindow w;
    w.show();
    w.InitData();
    return a.exec();
}
