#include "DictEngine.h"
#include <QDebug>

QMap<QString, QStringList> DictEngine::mCurrentEntriesTable;
DictEngine *DictEngine::mInstance = NULL;
DictEngine::GC DictEngine::gc;

DictEngine::DictEngine(QObject *parent) : QObject(parent){
}

DictEngine::~DictEngine(){

}

DictEngine* DictEngine::instance(){
    if(mInstance == NULL){
        mInstance = new DictEngine;
    }
    return mInstance;
}

QMap<QString, QStringList> DictEngine::searchTrans(QStringList words){
    QMap<QString, QStringList> result;
    QStringList entries;

    for(QString word : words){
        entries.clear();
        entries << "Trans 1" << "Trans 2" << "Trans 3 This is a message" << "Trans 4" << "Trans 5";
        result.insert(word, entries);
    }

    return result;
}
