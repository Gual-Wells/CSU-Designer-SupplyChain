#include "mainwindow.h"
#include <QApplication>
#include <QMouseEvent>
#include "edgedata.h"
#include "nodedata.h"
#include "global.cpp"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
