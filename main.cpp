#include "mainwindow.h"
#include "virtuallist.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    VirtualList w;
    QList<QString> list;

    for(int i=0; i<5000000; i++)
    {
        list.push_back(QString::number(i) + "");
    }
    w.Append(list);
    w.RefreshData();
    w.show();

    return a.exec();
}
