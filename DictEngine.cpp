#include "DictEngine.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QApplication>
#include <QJsonDocument>


static QString wiktionaryUrl = "https://en.wiktionary.org/wiki/";
static QString baiduDictUrl = "http://api.fanyi.baidu.com/api/trans/vip/translate";
static QString baiduAppId = "20170118000036209";
static QString baiduSecretKey = "koAkqialgJjn8SP6bLn7";

DictEngine *DictEngine::mInstance = NULL;
DictEngine::GC DictEngine::gc;

void FetchEntryPatchAsync::start(){
    if(obj){
        QStringList trans_list;
        for(int i = 0; i < words.length(); i++){
            trans_list = ((DictEngine*)obj)->fetchEntry(words[i], from, to);
            emit finishOne(words[i], trans_list);
        }
    }
    QThread::currentThread()->quit();
}

DictEngine::DictEngine(QObject *parent)
    : QObject(parent){
}

DictEngine::~DictEngine(){
}

DictEngine* DictEngine::instance(){
    if(mInstance == NULL){
        mInstance = new DictEngine;
    }
    return mInstance;
}

QStringList DictEngine::fetchEntry(QString word,
                                   QString from,
                                   QString to){
    QStringList trans_list;
    static Request request;
    Response response = request.get(buildURL(word, from, to).toStdString());
    if(response.errorCode() == 0 && response.statusCode() == 200){
        parseResponseMessage(QString::fromStdString(response.content()), NULL, &trans_list);
    }else{
        qDebug() << "failed to fetch for word:" << word;
        qDebug() << "\terror code:" << response.errorCode() <<" : error msg:" << response.errorMsg().c_str();
        qDebug() << "\tstatus code:" << response.statusCode();
    }
    return trans_list;
}

void DictEngine::fetchEntryPatchAsync(QStringList words,
                     QString from,
                     QString to){
    FetchEntryPatchAsync *worker = new FetchEntryPatchAsync;
    worker->obj = this;
    worker->words = words;
    worker->from = from;
    worker->to = to;
    connect(worker, &FetchEntryPatchAsync::finishOne, this, &DictEngine::on_requestFinished, Qt::QueuedConnection);
    mLauncher.asyncRun(worker, "start");
}

void DictEngine::on_requestFinished(QString word, QStringList trans_list){
    emit receivedEntryResponse(word, trans_list);
}

QString DictEngine::buildURL(const QString &word, const QString &from, const QString &to){
    QString salt = "35224047";
    QString joint = baiduAppId + word + salt + baiduSecretKey;

    static QCryptographicHash md5(QCryptographicHash::Md5);
    md5.reset();

    QByteArray bytes = joint.toUtf8();
    md5.addData(bytes);
    bytes = md5.result();
    QString sign = bytes.toHex().toLower();

    QString final_url = baiduDictUrl
            + "?q=" + word
            + "&from=" + from
            + "&to=" + to
            + "&appid=" + baiduAppId
            + "&salt=" + salt
            + "&sign=" + sign;
    return final_url;
}

void DictEngine::parseResponseMessage(const QString &message, QString *word, QStringList *trans){
    trans->clear();
    if(message.isEmpty()){
        return ;
    }
    QJsonDocument jsonDocument = jsonDocument = QJsonDocument::fromJson(message.toUtf8());
    if(jsonDocument.isEmpty() || jsonDocument.isNull()){
        return ;
    }
    QJsonObject root = jsonDocument.object();
    if(word != NULL && root.contains("word")){
        *word = root["word"].toString();
    }
    if(root.contains("trans_result")){
        QJsonArray list = root.take("trans_result").toArray();
        QJsonObject item;
        for(int i = 0; i < list.size(); i++){
            item = list.at(i).toObject();
            trans->append(item["dst"].toString());
        }
    }else if(root.contains("error_code")){
        qDebug() << "failed to fetch entry: " << root["error_msg"].toString();
        return;
    }
}

