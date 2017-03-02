#include "Launcher.h"

Launcher::Launcher(QObject *parent)
    :QThread(parent)
{

}

Launcher::~Launcher(){
    if(isRunning()){
        stopTasks();
    }
}

void Launcher::stopTasks(){
    mQuenedTasks.clear();
    terminate();
    wait();
}

void Launcher::asyncRun(QObject* worker, const char* funcName,
                     QGenericArgument val0,
                     QGenericArgument val1,
                     QGenericArgument val2,
                     QGenericArgument val3,
                     QGenericArgument val4,
                     QGenericArgument val5,
                     QGenericArgument val6,
                     QGenericArgument val7,
                     QGenericArgument val8,
                     QGenericArgument val9){
    QMutexLocker locker(&mMutex);
    if(this->isRunning()){
        Task task;
        task.worker = worker;
        task.funcName = funcName;
        task.val0 = val0;
        task.val1 = val1;
        task.val2 = val2;
        task.val3 = val3;
        task.val4 = val4;
        task.val5 = val5;
        task.val6 = val6;
        task.val7 = val7;
        task.val8 = val8;
        task.val9 = val9;
        mQuenedTasks.push_back(task);
        qDebug() << "push back one task";
        return;
    }
    worker->moveToThread(this);
    this->start();
    QMetaObject::invokeMethod(worker, funcName, Qt::QueuedConnection, QGenericReturnArgument(), val0,
                                           val1, val2, val3, val4, val5, val6, val7, val8, val9);
    connect(this, &Launcher::finished, this, &Launcher::on_finished, Qt::QueuedConnection);
}

void Launcher::on_finished(){
    QMutexLocker locker(&mMutex);
    if(mQuenedTasks.isEmpty()){
        return;
    }

    for(int i = 0; i < mQuenedTasks.size(); i++){
        Task task = mQuenedTasks[i];
        mQuenedTasks.removeAt(i);
        locker.unlock();
        asyncRun(task.worker, task.funcName,
                 task.val0,
                 task.val1,
                 task.val2,
                 task.val3,
                 task.val4,
                 task.val5,
                 task.val6,
                 task.val7,
                 task.val8,
                 task.val9);
        qDebug() << "pop one task";
    }
}
