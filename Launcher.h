#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

class Launcher : public QThread{
    Q_OBJECT
public:
    Launcher();
    void prepare(QObject* worker);

protected:
    void run(){
        exec();
    }
};
#endif // LAUNCHER_H
