#include "DictEngine.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QApplication>
#include <QJsonDocument>

#define TIMEOUT (30 * 1000)

static QString wiktionaryUrl = "https://en.wiktionary.org/wiki/";
static QString baiduDictUrl = "http://api.fanyi.baidu.com/api/trans/vip/translate";
static QString baiduAppId = "20170118000036209";
static QString baiduSecretKey = "koAkqialgJjn8SP6bLn7";

DictEngine *DictEngine::mInstance = NULL;
DictEngine::GC DictEngine::gc;

DictEngine::DictEngine(QObject *parent) : QObject(parent){
    connect(&networkAccessMgr2, &QNetworkAccessManager::finished, this, &DictEngine::on_requestFinished);
}

DictEngine::~DictEngine(){

}

DictEngine* DictEngine::instance(){
    if(mInstance == NULL){
        mInstance = new DictEngine;
    }
    return mInstance;
}

QStringList DictEngine::fetchEntry(const QString &word,
                                   const QString &from,
                                   const QString &to){

    QNetworkRequest request;
    request.setUrl(QUrl(buildURL(word, from, to)));
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);

    QNetworkReply *reply = networkAccessMgr1.get(request);
    reply->ignoreSslErrors();

    QTime time;
    time.start();

    bool isTimeout = false;

    while (!reply->isFinished()) {
        QApplication::processEvents();
        if (time.elapsed() >= TIMEOUT) {
            isTimeout = true;
            break;
        }
    }

    QString message;
    if (!isTimeout && reply->error() == QNetworkReply::NoError) {
        message = reply->readAll();
    }

    reply->deleteLater();

    QStringList trans_list;
    parseResponseMessage(message, NULL, &trans_list);
    return trans_list;
}

void DictEngine::fetchEntryAsync(const QString &word,
                       const QString &from,
                       const QString &to){
    QNetworkRequest request;
    request.setUrl(QUrl(buildURL(word, from, to)));
    networkAccessMgr2.get(request);
}

void DictEngine::on_requestFinished(QNetworkReply *reply){
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString message = (reply->error() == QNetworkReply::NoError) ? reply->readAll() : QString("network error, http status code[%1]").arg(code);

    QString word;
    QStringList trans;
    parseResponseMessage(message, &word, &trans);
    emit receivedEntryResponse(word, trans);

    reply->deleteLater();
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

