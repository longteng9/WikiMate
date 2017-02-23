#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include "Helper.h"
#include "DictEngine.h"
#include <QCryptographicHash>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Helper::instance()->mExecutableDirectory = QCoreApplication::applicationDirPath();

    MainWindow w;
    w.setWindowIcon(QIcon(":/static/app_icon_256.ico"));
    w.show();

    return a.exec();
}
