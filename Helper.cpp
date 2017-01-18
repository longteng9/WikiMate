#include "Helper.h"
#include <QDir>
#include <QVector>
#include <QStringList>
#include <QDebug>
#include <QFile>

Helper *Helper::mInstance = NULL;

Helper::Helper(QObject *parent) : QObject(parent)
{

}

Helper* Helper::instance(){
    if(Helper::mInstance == NULL){
        Helper::mInstance = new Helper;
    }
    return mInstance;
}

void Helper::scanFolder(const QString& path, QStringList *result, bool recur){
    QDir dir(path);
    if(!dir.exists()){
        return;
    }
    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsLast);
    QFileInfoList infoList = dir.entryInfoList();

    for(int i = 0; i < infoList.size(); i++){
        QFileInfo info = infoList.at(i);
        if(info.fileName() == "." || info.fileName() == ".."){
            continue;
        }
        if(info.isDir()){
            if(recur){
                scanFolder(info.filePath(), result);
            }else{
                QString path = info.absoluteFilePath();
                if(!path.endsWith('/') && !path.endsWith('\\')){
                    path = path.append('/');
                }
                result->append(path);
                qDebug() << path;
            }
        }else{
            result->append(info.absoluteFilePath());
        }
    }
}

QVector<QStringList> Helper::getWorkingFiles(const QString& path){
    QVector<QStringList> result;
    QStringList list;
    instance()->scanFolder(path, &list, false);
    for(auto iter = list.begin(); iter != list.end(); iter++){
        QFileInfo file(*iter);
        QStringList data;
        if(file.isDir()){
            data.append(":/static/question.png");
            data.append((*iter).mid(path.length() + 1));
            data.append("");
            data.append("");
            data.append("folder");
            data.append(" ");
            data.append("");
            data.append("1206-2-02");
        }else{
            data.append(":/static/question.png");
            data.append(file.fileName());
            data.append("123");
            data.append(QString::number(file.size()));
            data.append("translating");
            data.append("89");
            data.append("");
            data.append("1206-2-02");
        }
        result.append(data);
    }
    return result;
}

QString Helper::pathJoin(QString part1, QString part2){
    if(part1.contains('\\')){
        part1 = part1.replace('\\', '/');
    }
    if(!part1.endsWith('/')){
        part1 = part1.append('/');
    }
    if(part2.length() == 0){
        return part1;
    }
    if(part2.contains('\\')){
        part2 = part2.replace('\\', '/');
    }
    if(part2[0] == '/'){
        part2 = part2.remove(0, 1);
    }
    return part1 + part2;
}

