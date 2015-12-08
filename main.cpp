#include "mainwindow.h"
#include <QApplication>
#include "add_functions.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<movementData>("movementData");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
