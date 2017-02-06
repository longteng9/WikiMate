#include "FragmentManager.h"


FragmentManager *FragmentManager::mInstance = NULL;

FragmentManager::FragmentManager(QObject *parent) : QObject(parent)
{

}

FragmentManager* FragmentManager::instance(){
    if(FragmentManager::mInstance == NULL){
        FragmentManager::mInstance = new FragmentManager;
    }
    return mInstance;
}

QVector<QStringList> FragmentManager::buildFragments(const QStringList &sentences){
    QVector<QStringList> res;
    for(QString sentence: sentences){

    }
    return res;
}

QString FragmentManager::retrieveFragment(int block, int pos){
    return QString::number(pos);
}
