#include "Helper.h"
#include <QDir>
#include <QVector>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QFileDialog>
#include <QRegExp>
#include <QDataStream>
#include "FragmentManager.h"

Helper *Helper::mInstance = NULL;
Helper::GC Helper::gc;

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

/*
Project file JSON format:
{
    "tasks":[
        {
            "filename": "something",
            "words": 123,
            "size": 123,
            "status": "doing",
            "progress": 50,
            "tags": ""
            "date": "2017-01-25"
        }
    ]
}
*/
QVector<QStringList> Helper::getWorkingFiles(const QString& dirPath){
    QVector<QStringList> result;
    QString projFilename = Helper::instance()->pathJoin(dirPath, "project.meta");
    QFile file(projFilename);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Failed to open file: " << projFilename;
        return result;
    }
    QJsonParseError error;
    QByteArray data = file.readAll();
    if(data.isEmpty()){
        qDebug() << "Empty project file: " << projFilename;
        return result;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
    if (jsonDocument.isObject()) {
        QJsonObject root = jsonDocument.object();
        if (root.contains("tasks")){
            QJsonArray tasks = root.take("tasks").toArray();
            int size = tasks.size();
            QStringList data;
            for (int i = 0; i < size; i++) {
                data.clear();
                data.append(":/static/question.png");
                QJsonObject item = tasks.at(i).toObject();
                data.append(item.take("filename").toString());
                data.append(item.take("words").toString());
                data.append(item.take("size").toString());
                data.append(item.take("status").toString());
                data.append(item.take("progress").toString());
                data.append(item.take("tags").toString());
                data.append(item.take("date").toString());
                result.append(data);
            }
        }
    } else {
        qDebug() << "failed to parse JSON:" << error.errorString().toUtf8().constData();
    }
    return result;
}

void Helper::refreshWorkingDir(QString dirPath){
    // find project file in working directory, if doesn't exist, create one
    QString projFilename = Helper::instance()->pathJoin(dirPath, "project.meta");
    QFileInfo projFile = QFileInfo(projFilename);
    if (!projFile.exists()){
        QFile file(projFilename);
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    QFile file(projFilename);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Failed to open file: " << projFilename;
        return;
    }
    QJsonParseError error;
    QByteArray data = file.readAll();
    file.close();
    if(data.isEmpty()){
        data = QString("{\"tasks\": []}").toUtf8();
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
    QJsonObject root;
    if (jsonDocument.isObject()) {
        root = jsonDocument.object();
        QStringList existing_list;
        if (root.contains("tasks")){
            QJsonArray tasks = root.take("tasks").toArray();
            int size = tasks.size();
            for (int i = 0; i < size; ) {
                QJsonObject item = tasks.at(i).toObject();
                QString filename = item.take("filename").toString();
                QFile tmp(Helper::instance()->pathJoin(dirPath, filename));
                if (!tmp.exists()){
                    tasks.removeAt(i);
                    size--;
                }else{
                    i++;
                    existing_list.append(filename);
                }
            }

            QStringList list;
            instance()->scanFolder(dirPath, &list, false);

            for(auto iter = list.begin(); iter != list.end(); iter++){
                QFileInfo info(*iter);
                if (info.fileName() == "project.meta"
                        || info.fileName().endsWith(".wmtmp")
                        || info.fileName().startsWith("[EN]")){
                    continue;
                }
                bool existing = false;
                for(auto iter = existing_list.begin(); iter != existing_list.end(); iter++){
                    if(info.fileName() == *iter){
                        existing = true;
                        break;
                    }
                }
                if(existing){
                    continue;
                }

                if (!info.isDir()){

                    QJsonObject item;
                    item.insert("filename", info.fileName());
                    item.insert("words", QString::number(info.size() / 2));
                    item.insert("size", Helper::instance()->sciSize(info.size()));
                    item.insert("status", "DOING");
                    item.insert("progress", "0");
                    item.insert("tags", isUTF8File(info.absoluteFilePath()) ? "UTF-8" : "ANSI");
                    item.insert("date", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                    tasks.push_back(item);
                }
            }

            root["tasks"] = tasks;
        }
    } else {
        qDebug() << "failed to parse JSON:" << error.errorString().toUtf8().constData();
        return;
    }

    QFile file2(projFilename);
    if(!file2.open(QIODevice::WriteOnly)){
        qDebug() << "Failed to open file: " << projFilename;
        return;
    }

    jsonDocument.setObject(root);
    QByteArray bytes = jsonDocument.toJson(QJsonDocument::Indented);
    file2.write(bytes);
    file2.close();
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

QString Helper::sciSize(int64_t size){
    int64_t _1k = 1024;
    int64_t _1m = 1024 * _1k;
    int64_t _1g = 1024 * _1m;
    int64_t _1t = 1024 * _1g;

    if (size >= _1k && size < _1m){
       return QString::number(size / _1k, 'f', 2) + "K";
    }
    if (size >= _1m && size < _1g){
        return QString::number(size / _1m, 'f', 2) + "M";
    }
    if(size >= _1g && size < _1t){
        return QString::number(size / _1g, 'f', 2) + "G";
    }
    if(size >= _1t){
        return QString::number(size / _1t, 'f', 2) + "T";
    }
    return QString::number(size) + "B";
}

void Helper::addNewTasks(QWidget *parent){
    QStringList files = QFileDialog::getOpenFileNames(parent, "Add New Tasks", "./");
    if(!files.isEmpty()){
        for(QString name : files){
            Helper::instance()->copyFile(name, Helper::instance()->mProjectDirectory, false);
        }
        emit refreshTaskList();
    }
}

bool Helper::copyFile(const QString& absPath, const QString& newPath, bool overwrite){
    QFileInfo info(absPath);
    if(!info.exists()){
        return false;
    }
    QString newAbsPath = Helper::instance()->pathJoin(newPath, info.fileName());
    QFileInfo info2(newAbsPath);
    if(info2.exists() && overwrite){
        QFile::remove(newAbsPath);
    }
    return QFile::copy(absPath, newAbsPath);
}


bool Helper::isUTF8File(const QString& path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "failed to open file: " << file.errorString();
        return false;
    }
    QDataStream dataStream(&file);
    char* tmp = new char[1024];
    int length = dataStream.readRawData(tmp, 1024);
    file.close();
    unsigned char* data = (unsigned char*)tmp;
    unsigned char* end = data + length;

    bool isUTF8 = true;

    while(data){
        if(*data < 0x80){// (10000000): 值小于0x80的为ASCII字符
            data++;
        }else if(*data < 0xc0){// (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
            isUTF8 = false;
            break;
        }else if(*data < 0xe0){// (11100000): 此范围内为2字节UTF-8字符
            if(data >= end - 1){
                break;
            }
            if((data[1] & 0xc0) != 0x80){
                isUTF8 = false;
                break;
            }
            data += 2;
        }else if(*data < 0xf0){// (11110000): 此范围内为3字节UTF-8字符
            if(data >= end - 2){
                break;
            }
            if((data[1] & 0xc0) != 0x80 || (data[2] & 0xc0) != 0x80){
                isUTF8 = false;
                break;
            }
            data += 3;
        }else if(*data < 0xf8){// (11111000): 此范围内为4字节UTF-8字符
            if(data >= end - 3){
                break;
            }
            if((data[1] & 0xc0) != 0x80 || (data[2] & 0xc0) != 0x80 || (data[3] & 0xc0) != 0x80){
                isUTF8 = false;
                break;
            }
            data += 4;
        }else{
            isUTF8 = false;
            break;
        }
    }
    if(tmp){
        delete[] tmp;
    }
    return isUTF8;
}

bool Helper::equalStringList(const QStringList& list1, const QStringList& list2){
    if(list1.length() != list2.length()){
        return false;
    }

    for(int i = 0; i < list1.length(); i++){
        if(list1[i] != list2[i]){
            return false;
        }
    }
    return true;
}


void Helper::updateProjectFile(const QString& taskname, const QString& attr, const QString& value){
    QString sourceFilePath = FragmentManager::instance()->mSourceFilePath;
    QString projFilename = Helper::instance()->pathJoin(
                sourceFilePath.mid(0, sourceFilePath.lastIndexOf("/")),
                "project.meta");
    QFileInfo projFile = QFileInfo(projFilename);
    if (!projFile.exists()){
        return;
    }

    QFile file(projFilename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "failed to open file: " << projFilename;
        return;
    }
    QJsonParseError error;
    QByteArray data = file.readAll();
    file.close();
    if(data.isEmpty()){
        return;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
    QJsonObject root;
    if (jsonDocument.isObject()) {
        root = jsonDocument.object();
        if (root.contains("tasks")){
            QJsonArray tasks = root.take("tasks").toArray();
            int size = tasks.size();
            for (int i = 0; i < size; i++) {
                QJsonObject item = tasks.at(i).toObject();
                QString filename = item.value("filename").toString();
                if(filename == taskname){
                    tasks.removeAt(i);
                    item[attr] = value;
                    tasks.append(item);
                    break;
                }
            }
            root["tasks"] = tasks;
        }
    } else {
        qDebug() << "failed to parse JSON:" << error.errorString().toUtf8().constData();
        return;
    }

    QFile file2(projFilename);
    if(!file2.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Failed to open file: " << projFilename;
        return;
    }

    jsonDocument.setObject(root);
    QByteArray bytes = jsonDocument.toJson(QJsonDocument::Indented);
    file2.write(bytes);
    file2.close();
}
