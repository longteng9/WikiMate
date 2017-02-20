#ifndef DICTENGINE_H
#define DICTENGINE_H

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
    QMap<QString, QStringList> searchTrans(QStringList words);

signals:

public slots:

private:
    explicit DictEngine(QObject *parent = 0);
    DictEngine(const DictEngine&) = default;
    virtual ~DictEngine();
    DictEngine& operator=(const DictEngine&) = default;

public:
    static QMap<QString, QStringList> mCurrentEntriesTable;

private:
    static DictEngine *mInstance;
    static GC gc;
};

#endif // DICTENGINE_H
