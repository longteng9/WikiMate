#include "DictEngine.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QApplication>
#include <QJsonDocument>
#include <QFileInfo>
#include <QFile>
#include "MessageForm.h"
#include <QDir>

static QString wiktionaryDumpDir = "";
static QString wiktionaryDumpDB = "";
static QString transMemFilePath = "";

static QString wiktionaryUrl = "https://en.wiktionary.org/wiki/";

static QString baiduDictUrl = "http://api.fanyi.baidu.com/api/trans/vip/translate";
static QString baiduAppId = "20170118000036209";
static QString baiduSecretKey = "koAkqialgJjn8SP6bLn7";


DictEngine *DictEngine::mInstance = NULL;
DictEngine::GC DictEngine::gc;

#ifndef BLOCK_NET_ENTRY_QUERY
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

QString DictEngine::fetchWikiPage(QString word){
    return QString();
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
#endif

bool DictEngine::dataFileValid(){
    QFileInfo dumpDB_info(wiktionaryDumpDB);
    if(!dumpDB_info.exists()){
        return false;
    }
    QDir dumpPage_dir(wiktionaryDumpDir);
    if(!dumpPage_dir.exists()){
        return false;
    }
    return true;
}

void QueryDumpEntryPatchAsync::start(){
    if(obj){
        qDebug() << words;
        for(QString word : words){
            bool isDefined = false;

            QMap<QString, QString> result = DictEngine::instance()->queryWikiDumpEntry(word);
            if(result["en_entries"] != ""){
                QString en_entries = result["en_entries"];
                if(en_entries.contains("|")){
                    QStringList trans = en_entries.split("|", QString::SkipEmptyParts);
                    emit finishOne(word, trans);
                    isDefined = true;
                }else if(en_entries.contains(";")){
                    QStringList trans = en_entries.split(";", QString::SkipEmptyParts);
                    emit finishOne(word, trans);
                    isDefined = true;
                }else{
                    QStringList trans;
                    trans.append(en_entries);
                    emit finishOne(word, trans);
                    isDefined = true;
                }
            }else if(result["en_entries"] == "" && result["redirection"] != ""){
                QStringList directs = result["redirection"].split("|", QString::SkipEmptyParts);
                QMap<QString, QString> tmp;
                for(QString direct : directs){
                    tmp = DictEngine::instance()->queryWikiDumpEntry(direct);
                    if(tmp["en_entries"] != ""){
                        break;
                    }
                }
                if(tmp["en_entries"] != ""){
                    QString en_entries = tmp["en_entries"];
                    if(en_entries.contains("|")){
                        QStringList trans = en_entries.split("|", QString::SkipEmptyParts);
                        emit finishOne(word, trans);
                        isDefined = true;
                    }else if(en_entries.contains(";")){
                        QStringList trans = en_entries.split(";", QString::SkipEmptyParts);
                        emit finishOne(word, trans);
                        isDefined = true;
                    }else{
                        QStringList trans;
                        trans.append(en_entries);
                        emit finishOne(word, trans);
                        isDefined = true;
                    }
                }
            }
            if(!isDefined){
                emit finishOne(word, QStringList());
            }
        }
    }
    QThread::currentThread()->quit();
}

DictEngine::DictEngine(QObject *parent)
    : QObject(parent){
    wiktionaryDumpDir = QCoreApplication::applicationDirPath() + "/dict/wiktionary/pages_zh/";
    wiktionaryDumpDB = QCoreApplication::applicationDirPath() + "/dict/wiktionary/pages_zh.db";
    transMemFilePath = QCoreApplication::applicationDirPath() + "/dict/user_trans_mem.json";

    if(!dataFileValid()){
        qDebug() << "Data files are invalid:";
        qDebug() << "Wiktionary dump database: " << wiktionaryDumpDB
                 << "\nWiktionary dump pages dir:" << wiktionaryDumpDir;
        MessageForm::createAndShowAs(MessageForm::Role::QueryDialogForm,
                                     "Warning",
                                     "Wiktionary dump database and related files are missing or invalid, contact Xavier to get the data files and restart program.",
                                     [](bool val){
            qApp->quit();
        });
        return;
    }

    mDB = QSqlDatabase::addDatabase("QSQLITE");
    mDB.setDatabaseName(wiktionaryDumpDB);
    if(!mDB.open()){
        qDebug() <<"failed to open database: " << wiktionaryDumpDB;
    }else{
        qDebug() <<"succeed to open database: " << wiktionaryDumpDB;
    }
}

DictEngine::~DictEngine(){
    if(mDB.isOpen()){
        mDB.close();
        qDebug() << "succeed to close database: " << wiktionaryDumpDB;
    }
}

DictEngine* DictEngine::instance(){
    if(mInstance == NULL){
        mInstance = new DictEngine;
    }
    return mInstance;
}

QMap<QString, QString> DictEngine::queryWikiDumpEntry(QString word){
    QMap<QString, QString> result;
    if(!mDB.isOpen()){
        if(!mDB.open()){
            qDebug() <<"failed to open database: " << wiktionaryDumpDB;
            return result;
        }
    }

    QSqlQuery query;
    if(query.exec(QString("SELECT * from pages_zh WHERE page_title=\"%1\"").arg(word))){
        while(query.next()){
            result["page_id"] = query.value("page_id").toString();
            result["page_title"] = word;
            result["redirection"] = query.value("redirection").toString();
            result["en_entries"] = query.value("en_entries").toString();
            result["tag"] = query.value("tag").toString();

        }
    }

    return result;
}

QVector<QMap<QString, QString> > DictEngine::queryWikiDumpEntryFuzzy(QString word){
    QVector<QMap<QString, QString> > result;
    if(!mDB.isOpen()){
        if(!mDB.open()){
            qDebug() <<"failed to open database: " << wiktionaryDumpDB;
            return result;
        }
    }

    QSqlQuery query;
    if(query.exec(QString("SELECT * from pages_zh WHERE page_title LIKE \"%" + word + "%\""))){
        while(query.next()){
            QMap<QString, QString> item;
            item["page_id"] = query.value("page_id").toString();
            item["page_title"] = query.value("page_title").toString();
            item["redirection"] = query.value("redirection").toString();
            item["en_entries"] = query.value("en_entries").toString();
            item["tag"] = query.value("tag").toString();
            result.append(item);
        }
    }

    return result;
}


void DictEngine::stopQueryAndFetch(){
    mLauncher.stopTasks();
}

QString DictEngine::queryWikiPageById(QString pageId){
    QString path = wiktionaryDumpDir + pageId + ".wiki";
    QFileInfo info(path);
    if(!info.exists()){
        return "";
    }
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return "";
    }
    QByteArray bytes = file.readAll();
    QString result = bytes;
    return result;
}

void DictEngine::queryDumpEntryPatchAsync(QStringList words,
                     QString from,
                     QString to){
    QueryDumpEntryPatchAsync *worker = new QueryDumpEntryPatchAsync;
    worker->obj = this;
    worker->words = words;
    worker->from = from;
    worker->to = to;
    connect(worker, &QueryDumpEntryPatchAsync::finishOne, this, &DictEngine::on_queryFinished, Qt::QueuedConnection);
    mLauncher.asyncRun(worker, "start");
}

void DictEngine::on_queryFinished(QString word, QStringList trans_list){
    emit queryDumpEntryFinished(word, trans_list);
}

/*
Trans-Mem file JSON format:
{
    "trans-mems":[
        {
            "word": "something",
            "defines": "abcd/#/abcd/#/abcd"
        },{...}
    ]
}
*/
void DictEngine::insertTransMem(const QString& word, const QString& define){
    if(word.isEmpty() || define.isEmpty()){
        return;
    }
    QFile transMemFile(transMemFilePath);
    if(!transMemFile.open(QIODevice::ReadWrite | QIODevice::Text)){
        qDebug() << "failed to open file: " << transMemFilePath;
        return;
    }
    QJsonParseError error;
    QByteArray data = transMemFile.readAll();
    transMemFile.close();
    if(data.isEmpty()){
        data = QString("{\"trans-mems\":[]}").toUtf8();
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
    QJsonObject root = jsonDocument.object();
    if (root.contains("trans-mems")){
        QJsonArray transMems = root.take("trans-mems").toArray();
        int size = transMems.size();
        bool updated = false;
        for(int i = 0; i < size; i++){
            if(transMems.at(i).toObject().value("word").toString() == word){
                updated = true;
                QJsonObject item = transMems.at(i).toObject();
                item["defines"] = item["defines"].toString() + "/#/" + define;
                transMems.removeAt(i);
                transMems.append(item);
                break;
            }
        }
        if(!updated){
            QJsonObject item{{"word", word}, {"defines", define}};
            transMems.append(item);
        }
        root["trans-mems"] = transMems;
    }

    QFile file2(transMemFilePath);
    if(!file2.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Failed to open file: " << transMemFilePath;
        return;
    }

    jsonDocument.setObject(root);
    QByteArray bytes = jsonDocument.toJson(QJsonDocument::Indented);
    file2.write(bytes);
    file2.close();
}

QMap<QString, QStringList> DictEngine::getAllTransMem(){
    QMap<QString, QStringList> result;
    QFile transMemFile(transMemFilePath);
    if(!transMemFile.open(QIODevice::ReadWrite | QIODevice::Text)){
        qDebug() << "failed to open file: " << transMemFilePath;
        return result;
    }
    QJsonParseError error;
    QByteArray data = transMemFile.readAll();
    transMemFile.close();
    if(data.isEmpty()){
        return result;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
    QJsonObject root = jsonDocument.object();
    if (root.contains("trans-mems")){
        QJsonArray transMems = root.take("trans-mems").toArray();
        QJsonObject item;
        int size = transMems.size();
        for(int i = 0; i < size; i++){
            item = transMems.at(i).toObject();
            result.insert(item["word"].toString(), item["defines"].toString().split("/#/", QString::KeepEmptyParts));
        }
    }
    return result;
}

void DictEngine::eraseTransMem(const QString& word){

}

void DictEngine::eraseTransMemAll(){
    QFileInfo info(transMemFilePath);
    if(!info.exists()){
        return;
    }
    QFile file(transMemFilePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "failed to open user trans-mem file:" << transMemFilePath;
        return;
    }
    file.write(QString("{\"trans-mems\": []}").toUtf8());
    file.close();
    qDebug() << "erase all items in trans-mem:"<<transMemFilePath;
}
