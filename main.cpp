#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include "Helper.h"
#include <QTextBrowser>
#include "Request.h"
#include "DictEngine.h"
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "qApp->applicationFilePath():" << qApp->applicationFilePath();
    qDebug() << "QCoreApplication::applicationDirPath():" << QCoreApplication::applicationDirPath();
    qDebug() << "QDir::currentPath():" << QDir::currentPath();
    qDebug() << "QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation):" << QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    qDebug() << "QStandardPaths::writableLocation(QStandardPaths::AppDataLocation):" <<QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << "QStandardPaths::writableLocation(QStandardPaths::TempLocation):" <<QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    
    MainWindow w;
    w.setWindowIcon(QIcon(":/static/app_icon_256.ico"));
    w.show();

    return a.exec();
}
