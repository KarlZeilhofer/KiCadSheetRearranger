#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("zeilhofer.co.at");
    QCoreApplication::setApplicationName("KiCadSheetRearranger");

    MainWindow* w;
    if(argc == 1){
        w = new MainWindow();
    }else{
        w = new MainWindow(0,QString(argv[1]));
    }

    w->show();

    int ret = a.exec();

    w->~MainWindow();
    return ret;
}
