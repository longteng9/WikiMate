#ifndef DICTENGINE_H
#define DICTENGINE_H

#include <QSettings>
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QString>
#include <QVector>
#include <QThread>
#include "Request.h"
#include "Launcher.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlField>

class DictEngine;

#ifndef BLOCK_NET_ENTRY_QUERY
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
#endif

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
                //delete mInstance;
                //mInstance = NULL;
            }
        }
    };
public:
    static DictEngine* instance();
    bool dataFileValid();
#ifndef BLOCK_NET_ENTRY_QUERY
    QStringList fetchEntry(QString word,
                           QString from,
                           QString to);
    void fetchEntryPatchAsync(QStringList words,
                         QString from,
                         QString to);
    QString fetchWikiPage(QString word);
#endif
    void queryDumpEntryPatchAsync(QStringList words,
                         QString from,
                         QString to);
    QMap<QString, QString> queryWikiDumpEntry(QString word);
    QVector<QMap<QString, QString> > queryWikiDumpEntryFuzzy(QString word);
    QString queryWikiPageById(QString pageId);

    void eraseTransMem(const QString& word);
    void eraseTransMemAll();
    void insertTransMem(const QString& word, const QString& define);
    QMap<QString, QStringList> getAllTransMem();
    void stopQueryAndFetch();
    void exposeTMForJieba();

public slots:
#ifndef BLOCK_NET_ENTRY_QUERY
    void on_requestFinished(QString word, QStringList trans_list);
#endif
    void on_queryFinished(QString word, QStringList trans_list);

signals:
    void receivedEntryResponse(QString word, QStringList trans);
    void queryDumpEntryFinished(QString word, QStringList trans);

private:
    explicit DictEngine(QObject *parent = 0);
    DictEngine(const DictEngine&) = default;
    virtual ~DictEngine();
    DictEngine& operator=(const DictEngine&) = default;
#ifndef BLOCK_NET_ENTRY_QUERY
    QString buildURL(const QString &word,
                     const QString &from,
                     const QString &to);
    void parseResponseMessage(const QString &message, QString* word, QStringList *trans);
#endif

private:
    static DictEngine *mInstance;
    static GC gc;
    Launcher mLauncher;
    QSqlDatabase mDB;
};

#endif // DICTENGINE_H
