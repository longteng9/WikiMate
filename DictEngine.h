#ifndef DICTENGINE_H
#define DICTENGINE_H

#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QString>

class DictEngine : public QObject
{
    Q_OBJECT
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
    QStringList fetchEntry(const QString &word,
                           const QString &from,
                           const QString &to);
    void fetchEntryAsync(const QString &word,
                           const QString &from,
                           const QString &to);

signals:
    void receivedEntryResponse(QString word, QStringList trans);

public slots:
    void on_requestFinished(QNetworkReply *reply);

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
    QNetworkAccessManager networkAccessMgr1;
    QNetworkAccessManager networkAccessMgr2;
};

#endif // DICTENGINE_H
