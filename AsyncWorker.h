#ifndef ASYNCWORKER_H
#define ASYNCWORKER_H

#include <QObject>
#include <functional>

class AsyncWorker : public QObject
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
    static AsyncWorker* instance();
    void postTask(){}

signals:
    void fragmentDataReady();

public slots:

private:
    explicit AsyncWorker(QObject *parent = 0);
    AsyncWorker(const AsyncWorker&) = default;
    virtual ~AsyncWorker();
    AsyncWorker& operator=(const AsyncWorker&) = default;

private:
    static AsyncWorker *mInstance;
    static GC gc;
};

#endif // ASYNCWORKER_H
