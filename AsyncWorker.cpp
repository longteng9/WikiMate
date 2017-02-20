#include "AsyncWorker.h"

AsyncWorker *AsyncWorker::mInstance = NULL;
AsyncWorker::GC AsyncWorker::gc;

AsyncWorker::AsyncWorker(QObject *parent) : QObject(parent)
{

}

AsyncWorker::~AsyncWorker(){

}

AsyncWorker* AsyncWorker::instance(){
    if(mInstance == NULL){
        mInstance = new AsyncWorker;
    }
    return mInstance;
}


