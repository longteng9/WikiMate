#ifndef TRANSMEMORY_H
#define TRANSMEMORY_H

#include <QObject>
#include <QString>
#include <QMap>

class TransMemory : public QObject
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
    static TransMemory* instance();
    void deleteEntry(const QString& word);
    void updateEntry(const QString& word, const QString value);
    QString queryEntry(const QString& word);

signals:

public slots:

private:
    explicit TransMemory(QObject *parent = 0);
    TransMemory(const TransMemory&) = default;
    virtual ~TransMemory();
    TransMemory& operator=(const TransMemory&) = default;

    void loadDict(const QString& path);
    void dumpDict(const QString& path);
    void exposeToJieba();

private:
    static TransMemory *mInstance;
    static GC gc;
    QString mTransMemPath;
    QMap<QString, QString> mTransMem;
};

#endif // TRANSMEMORY_H
