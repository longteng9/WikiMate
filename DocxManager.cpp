#include "DocxManager.h"
#include "Archive.h"
#include <QStandardPaths>
#include "Helper.h"
#include <QFile>
#include <QDebug>

DocxManager::DocxManager(QObject *parent)
    : QObject(parent)
{

}

int DocxManager::getWordCount(const QString& path){
    zip::Archive archive;
    std::string dup = path.toStdString();
    dup = dup.substr(dup.find_last_of("\\/")+1);
    dup = dup.substr(0, dup.find_last_of("."));
    int err = archive.uncompress(path.toStdString(),
                                 Helper::instance()->pathJoin(QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace("\\", "/"), QString(dup.c_str())).toStdString());
    if(err != 0){
        qDebug() <<"failed to uncompress:" << path;
        return -1;
    }

    QString propPath = Helper::instance()->pathJoin(QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace("\\", "/"), QString(dup.c_str())) + "/docProps/app.xml";
    QFile file(propPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "failed to open: " << propPath;
        return -1;
    }
    QString content = file.readAll();
    int start = content.indexOf("<Words>");
    int end = content.indexOf("</Words>", start);
    QString strNum = content.mid(start + 7, end - (start + 7));
    return strNum.toInt();
}

QStringList DocxManager::getFragmentList(const QString& path){
    QStringList results;

    return results;
}
