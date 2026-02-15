#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("FastPay");
    a.setOrganizationName("FastPay");
    MainWindow w;
    w.show();
    return a.exec();
}
