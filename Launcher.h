#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

class Launcher : public QThread{
    Q_OBJECT
    class Task{
    public:
        QObject* worker;
        const char* funcName;
        QGenericArgument val0 = QGenericArgument(Q_NULLPTR);
        QGenericArgument val1 = QGenericArgument();
        QGenericArgument val2 = QGenericArgument();
        QGenericArgument val3 = QGenericArgument();
        QGenericArgument val4 = QGenericArgument();
        QGenericArgument val5 = QGenericArgument();
        QGenericArgument val6 = QGenericArgument();
        QGenericArgument val7 = QGenericArgument();
        QGenericArgument val8 = QGenericArgument();
        QGenericArgument val9 = QGenericArgument();
    };
public:
    Launcher(QObject *parent = 0);
    void asyncRun(QObject* worker, const char* funcName,
                         QGenericArgument val0 = QGenericArgument(Q_NULLPTR),
                         QGenericArgument val1 = QGenericArgument(),
                         QGenericArgument val2 = QGenericArgument(),
                         QGenericArgument val3 = QGenericArgument(),
                         QGenericArgument val4 = QGenericArgument(),
                         QGenericArgument val5 = QGenericArgument(),
                         QGenericArgument val6 = QGenericArgument(),
                         QGenericArgument val7 = QGenericArgument(),
                         QGenericArgument val8 = QGenericArgument(),
                         QGenericArgument val9 = QGenericArgument());

protected:
    void run(){
        exec();
    }

    void on_finished();

private:
    QVector<Task> mQuenedTasks;
    QMutex mMutex;
};
#endif // LAUNCHER_H
