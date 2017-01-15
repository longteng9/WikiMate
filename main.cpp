#include "MainWindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowIcon(QIcon(":/static/app_icon_256.ico"));
    w.show();

    return a.exec();
}
