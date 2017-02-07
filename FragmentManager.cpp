#include "FragmentManager.h"
#include <QFile>
#include <QProcess>
#include <QStringList>
#include <QDebug>
#include <QMessageBox>
#include "MainWindow.h"

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

void PythonThread::run(){
    QProcess process;
    process.start(mCmdIn);
    process.waitForStarted();
    process.waitForFinished();
    mStdout = process.readAllStandardOutput();
    mStderr = process.readAllStandardError();
}


QVector<QStringList> FragmentManager::buildFragments(const QStringList &sentences){
    QVector<QStringList> res;
    QFile text_in("mandarin_parse_in.txt");
    text_in.open(QIODevice::ReadWrite);
    for(QString sentence: sentences){
        text_in.write(sentence.toUtf8() + "\n");
    }
    text_in.close();

    QMessageBox *messageBox = new QMessageBox(QMessageBox::NoIcon,
                                              "Please Wait...",
                                              "Backend is processing",
                                              QMessageBox::Close,
                                              NULL);
    messageBox->show();
    PythonThread pythonThread;
    pythonThread.start();
    pythonThread.wait();
    messageBox->accept();
    delete messageBox;

    QFile text_out("mandarin_parse_out.txt");
    text_out.open(QIODevice::ReadOnly);
    QTextStream in(&text_out);
    in.setCodec("UTF-8");
    mCurrentTaskBlocksFragments.clear();
    QStringList lst;
    while(!in.atEnd()) {
        QString line = in.readLine();
        lst.clear();
        lst = line.split("/#/", QString::SkipEmptyParts);
        mCurrentTaskBlocksFragments.append(lst);
    }
    text_out.close();

    qDebug() << "python: " <<pythonThread.mStdout;
    return res;
}

bool FragmentManager::retrieveWord(QString word){
    for(QStringList lst : mCurrentTaskBlocksFragments){
        for(QString item : lst){
            if(item == word){
                return true;
            }
        }
    }
    return false;
}

QString FragmentManager::retrieveFragment(int block, int pos){
    return QString::number(pos);
}
