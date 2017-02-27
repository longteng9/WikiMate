#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include "Helper.h"
#include <QTextBrowser>
#include "Request.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Helper::instance()->mExecutableDirectory = QCoreApplication::applicationDirPath();
QTextBrowser b;
Request r;
Response re = r.get("https://www.wiktionary.org/");
qDebug() << re.statusCode();
b.setHtml(re.content().c_str());
b.show();

    MainWindow w;
    w.setWindowIcon(QIcon(":/static/app_icon_256.ico"));
    w.show();

    return a.exec();
}
