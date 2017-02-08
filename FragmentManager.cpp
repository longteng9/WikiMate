#include "FragmentManager.h"
#include <QFile>
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

/*
Python modules needed:
jieba
*/
void PythonThread::run(){
    QProcess process;
    process.start("C:/Python36/python.exe mandarin.py");
    if(!process.waitForStarted()){
        code = -1;
        return;
    }
    process.closeWriteChannel();
    process.waitForFinished();
    mStdout = process.readAllStandardOutput();
    mStderr = process.readAllStandardError();

    qDebug() << "code: " << code;
    qDebug() << "python stderr: " <<mStderr;
    qDebug() << "python stdout: " <<mStdout;
}


QVector<QStringList> FragmentManager::buildFragments(const QStringList &sentences){
    QVector<QStringList> res;
    QFile text_in("mandarin_parse_in.txt");
    text_in.open(QIODevice::ReadWrite);
    for(QString sentence: sentences){
        text_in.write(sentence.toUtf8() + "\n");
    }
    text_in.close();

    PythonThread pythonThread;
    pythonThread.start();
    pythonThread.wait();

    if(pythonThread.code < 0){
        qDebug() << "failed to start subprocess: code(" << pythonThread.code << ")";
        QMessageBox::warning(MainWindow::widgetRef, "Warning [" + QString::number(pythonThread.code) + "]", "Can't find python interpreter in PATH,\nensure you add it into system PATH", QMessageBox::NoButton, QMessageBox::Yes);
        return res;
    }

    QFile text_out("mandarin_parse_out.txt");
    if(text_out.open(QIODevice::ReadOnly)){
        QTextStream in(&text_out);
        in.setCodec("UTF-8");
        mBlocksFragments.clear();
        QStringList lst;
        while(!in.atEnd()) {
            QString line = in.readLine();
            lst.clear();
            lst = line.split("/#/", QString::SkipEmptyParts);
            mBlocksFragments.append(lst);
        }
        text_out.close();
    }else{
        qDebug() << "failed to open mandarin_parse_out.txt";
    }

    return res;
}

bool FragmentManager::retrieveWord(QString word){
    for(QStringList lst : mBlocksFragments){
        for(QString item : lst){
            if(item == word){
                return true;
            }
        }
    }
    return false;
}

QString FragmentManager::retrieveFragment(int block, int pos, int* word_begin, int* word_len){
    if(mBlocksFragments.isEmpty() || mBlocksFragments.size() <= block){
        return "";
    }


    QStringList frags = mBlocksFragments[block];
    int count = 0;
    for(QString word : frags){
        count += word.size();
        if(count > pos){
            *word_len = word.size();
            *word_begin = count - *word_len;
            return word;
        }
    }
    *word_begin = 0;
    *word_len = 0;
    return "";
}
