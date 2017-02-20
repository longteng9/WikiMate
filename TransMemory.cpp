#include "TransMemory.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QDebug>
#include "FragmentManager.h"

TransMemory* TransMemory::mInstance = NULL;
TransMemory::GC TransMemory::gc;

TransMemory::TransMemory(QObject *parent) : QObject(parent){
    loadDict(mTransMemPath);
}

TransMemory::~TransMemory(){
}

TransMemory* TransMemory::instance(){
    if(mInstance == NULL){
        mInstance = new TransMemory;
    }
    return mInstance;
}

void TransMemory::deleteEntry(const QString& word){
    if(mTransMem.contains(word)){
        mTransMem.remove(word);
    }
    dumpDict(mTransMemPath);
}

void TransMemory::updateEntry(const QString& word, const QString value){
    if(mTransMem.contains(word)){
        mTransMem[word] = value;
    }else{
        mTransMem.insert(word, value);
    }
    dumpDict(mTransMemPath);
}

QString TransMemory::queryEntry(const QString& word){
    if(mTransMem.contains(word)){
        return mTransMem[word];
    }else{
        return "";
    }
}

void TransMemory::loadDict(const QString& path){
    mTransMem.clear();
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "failed to open file: " << path;
        return;
    }
    QJsonParseError error;
    QByteArray data = file.readAll();
    file.close();
    if(data.isEmpty()){
        return;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
    QJsonObject root = jsonDocument.object();

    if(root.contains("entries")){
        QJsonArray entries = root.take("entries").toArray();
        int size = entries.size();
        for (int i = 0; i < size; i++){
            QJsonObject entry = entries.at(i).toObject();
            if(entry.contains("word") && entry.contains("trans")){
                mTransMem.insert(entry["word"].toString(), entry["trans"].toString());
            }
        }
    }
}

void TransMemory::dumpDict(const QString& path){
    QJsonDocument jsonDocument;
    QJsonObject root;
    QJsonArray entries;
    QJsonObject item;

    for(QString key : mTransMem.keys()){
        item["word"] = key;
        item["trans"] = mTransMem[key];
        entries.append(item);
    }
    root["entries"] = entries;

    jsonDocument.setObject(root);
    QByteArray bytes = jsonDocument.toJson(QJsonDocument::Indented);

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "failed to open file for writing TransMem";
        return;
    }
    file.write(bytes);
    file.close();

    exposeToJieba();
}

void TransMemory::exposeToJieba(){
    QFile file(JiebaPaths::user_dict);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "failed to open file for writing user.dict.utf8";
        return;
    }

    for(QString word : mTransMem.keys()){
        word += "\n";
        file.write(word.toUtf8());
    }
    file.close();
}
