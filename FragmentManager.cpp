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
    process.start("python.exe D:\\build-WikiMate-Desktop_Qt_5_8_0_MinGW_32bit-Debug\\debug\\mandarin.py");
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


void FragmentManager::buildFragments(const QString &path){
    QStringList sentences;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return ;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();

    int pre = 0;
    int pos = content.indexOf(QRegExp("\u3002|\uff01|\uff1f"), pre);

    while (pos >= 0){
        sentences.append(content.mid(pre, pos - pre + 1).trimmed());
        pre = pos + 1;
        pos = content.indexOf(QRegExp("\u3002|\uff01|\uff1f"), pre);
    }
    if(!content.endsWith("。") && !content.endsWith("！") && !content.endsWith("？")){
        sentences.append(content.mid(pre));
    }

    mFragmentList = sentences;
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
        return;
    }

    mFragmentList = sentences;

    QFile text_out("mandarin_parse_out.txt");
    if(text_out.open(QIODevice::ReadOnly)){
        QTextStream in(&text_out);
        in.setCodec("UTF-8");
        mFragmentWordList.clear();
        while(!in.atEnd()) {
            QString line = in.readLine();
            mFragmentWordList.append(line.split("/#/", QString::SkipEmptyParts));
        }
        text_out.close();
    }else{
        qDebug() << "failed to open mandarin_parse_out.txt";
    }
}

bool FragmentManager::retrieveWord(QString word){
    for(QStringList lst : mFragmentWordList){
        for(QString item : lst){
            if(item == word){
                return true;
            }
        }
    }
    return false;
}

QStringList FragmentManager::currentBlockFragments(){
    return mFragmentWordList[mCurrentIndex];
}
