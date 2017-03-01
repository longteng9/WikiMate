#ifndef DICTENGINE_H
#define DICTENGINE_H

#include <QSettings>
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QString>
#include <QThread>
#include "Request.h"
#include "Launcher.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlField>

class DictEngine;
class FetchEntryPatchAsync : public QObject{
    Q_OBJECT
signals:
    void finishOne(QString word, QStringList trans_list);

public slots:
    void start();

public:
    DictEngine *obj;
    QStringList words;
    QString from;
    QString to;
};

class QueryDumpEntryPatchAsync : public QObject{
    Q_OBJECT
signals:
    void finishOne(QString word, QStringList trans_list);

public slots:
    void start();

public:
    DictEngine *obj;
    QStringList words;
    QString from;
    QString to;
};


class DictEngine : public QObject
{
    Q_OBJECT
    friend class FetchEntryAsync;
    class GC{
    public:
        ~GC(){
            if(mInstance != NULL){
                delete mInstance;
                mInstance = NULL;
            }
        }
    };
public:
    static DictEngine* instance();
    QStringList fetchEntry(QString word,
                           QString from,
                           QString to);
    void fetchEntryPatchAsync(QStringList words,
                         QString from,
                         QString to);
    void queryDumpEntryPatchAsync(QStringList words,
                         QString from,
                         QString to);
    QMap<QString, QString> queryWikiDumpEntry(QString word);
    QString fetchWikiPage(QString word);

public slots:
    void on_requestFinished(QString word, QStringList trans_list);
    void on_queryFinished(QString word, QStringList trans_list);

signals:
    void receivedEntryResponse(QString word, QStringList trans);
    void queryDumpEntryFinished(QString word, QStringList trans);

private:
    explicit DictEngine(QObject *parent = 0);
    DictEngine(const DictEngine&) = default;
    virtual ~DictEngine();
    DictEngine& operator=(const DictEngine&) = default;

    QString buildURL(const QString &word,
                     const QString &from,
                     const QString &to);
    void parseResponseMessage(const QString &message, QString* word, QStringList *trans);


private:
    static DictEngine *mInstance;
    static GC gc;
    Launcher mLauncher;
    QSqlDatabase mDB;
};

#endif // DICTENGINE_H
