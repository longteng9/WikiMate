#include "FragmentManager.h"
#include <QFile>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QDomElement>
#include <QtMath>
#include <QDomDocument>
#include "MainWindow.h"
#include "Helper.h"
#include "cppjieba/Jieba.hpp"
#include <vector>
#include <string>

const char* JiebaPaths::jieba_dict = "dict/jieba.dict.utf8";
const char* JiebaPaths::user_dict = "dict/user.dict.utf8";
const char* JiebaPaths::hmm_model = "dict/hmm_model.utf8";
const char* JiebaPaths::idf = "dict/idf.utf8";
const char* JiebaPaths::stop_words = "dict/stop_words.utf8";

FragmentManager *FragmentManager::mInstance = NULL;
FragmentManager::GC FragmentManager::gc;

FragmentManager::FragmentManager(QObject *parent) : QObject(parent){
    mJieba = new cppjieba::Jieba(JiebaPaths::jieba_dict,
                                 JiebaPaths::hmm_model,
                                 JiebaPaths::user_dict,
                                 JiebaPaths::idf,
                                 JiebaPaths::stop_words);
}

FragmentManager::~FragmentManager(){
    if(mJieba != NULL){
        delete mJieba;
        mJieba = NULL;
    }
}

FragmentManager* FragmentManager::instance(){
    if(FragmentManager::mInstance == NULL){
        FragmentManager::mInstance = new FragmentManager;
    }
    return mInstance;
}

void FragmentManager::buildOrLoadFragments(const QString &path){
    QString name = path.mid(path.lastIndexOf("/")+1);
    for(QStringList line : Helper::instance()->mTaskListBackup){
        if(line[1] == name){
            if(line[5] != "0"){
                loadRecords(path + ".wmtmp");
                return;
            }
        }
    }
    buildFragments(path);
}

void FragmentManager::buildFragments(const QString &path){
    mFragmentList.clear();
    mFragmentTransList.clear();
    mFragmentWordList.clear();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "failed to open source file for building fragments";
        return ;
    }
    mSourceFilePath = path;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();

    int pre = 0;
    int pos = content.indexOf(QRegExp("\u3002|\uff01|\uff1f"), pre);

    // 构建mFragmentList
    while (pos >= 0){
        QString frag = content.mid(pre, pos - pre + 1).trimmed();
        // frag = frag.replace("\n", "");
        mFragmentList.append(frag);
        pre = pos + 1;
        pos = content.indexOf(QRegExp("\u3002|\uff01|\uff1f"), pre);
    }
    if(!content.endsWith("。")
            && !content.endsWith("！")
            && !content.endsWith("？")){
        mFragmentList.append(content.mid(pre));
    }

    // 构建mFragmentTransList
    for(int i = 0; i < mFragmentList.length(); i++){
        mFragmentTransList.append("");
    }


    // 构建mFragmentWordList
    for(int i = 0; i < mFragmentList.length(); i++){
        mFragmentWordList.append(cutWords(mFragmentList.at(i)));
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

QStringList FragmentManager::currentFragmentWords(){
    return mFragmentWordList[mCurrentIndex];
}

QString FragmentManager::getFormatContent(){
    QString result = "";
    for(int i = 0; i < mFragmentList.size(); i++){
        if(i == mCurrentIndex){
            result += "<strong style=\"color:red; background:white\">[Frag:" + QString::number(mCurrentIndex) + "]</strong><span style=\"background:#4FBDFE\">";
            result += mFragmentList[i] + "</span>";
            if(mFragmentTransList[i] != ""){
                result += "<br><span style=\"background:#F0F0F0\">=>" + mFragmentTransList[i] + "</span>";
            }
            result += "<br><br>";
            continue;
        }
        result += mFragmentList[i];
        if(mFragmentTransList[i] != ""){
            result += "<br><span style=\"background:#F0F0F0\">=>" + mFragmentTransList[i] + "</span>";
        }
        result += "<br><br>";
    }
    return result;
}

void FragmentManager::updateFragmentTrans(const QString& trans){
    if(mCurrentIndex >= mFragmentTransList.length()){
        return;
    }
    mFragmentTransList[mCurrentIndex] = trans;
    flushRecords();

    int translatedCount = 0;
    for(int i = 0; i < mFragmentTransList.length(); i++){
        if(!mFragmentTransList[i].isEmpty()){
            translatedCount++;
        }
    }
    Helper::instance()->updateProjectFile(mSourceFilePath.mid(mSourceFilePath.lastIndexOf("/")+1),
                                          "progress",
                                          QString::number(qCeil((double)(translatedCount) / mFragmentList.length() * 100)));
}

void FragmentManager::loadRecords(const QString &path){
    mFragmentList.clear();
    mFragmentTransList.clear();
    mFragmentWordList.clear();
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        qDebug() << "failed to open .wmtmp file";
        return;
    }
    QString error;
    int errorLine;
    int errorCol;

    QDomDocument doc;
    if(!doc.setContent(&file, false, &error, &errorLine, &errorCol)){
        qDebug() << "failed to set content to QDomDocument instance";
        file.close();
        return;
    }

    QDomElement root = doc.documentElement();
    mSourceFilePath = root.attribute("source");

    QDomNode node = root.firstChild();
    while(!node.isNull()){
        if(node.isElement()){
            QDomElement fragment = node.toElement();
            QDomNodeList list = fragment.childNodes();
            int id = fragment.attribute("id").toInt();

            bool hasTrans = false;
            for(int i = 0; i < list.count(); i++){
                QDomNode child = list.at(i);
                if(child.isElement()){
                    QDomElement element = child.toElement();
                    if(element.tagName() == "text"){
                        mFragmentList.append(element.text());
                    }else if(element.tagName() == "words"){
                        mFragmentWordList.append(element.text().split("/#/", QString::SkipEmptyParts));
                    }else if(element.tagName() == "trans"){
                        mFragmentTransList.append(element.text());
                        hasTrans = true;
                    }
                }
            }
            if(!hasTrans){
                mFragmentTransList.append("");
            }
        }
        node = node.nextSibling();
    }
}

void FragmentManager::flushRecords(){
    QDomDocument doc;
    QDomElement root = doc.createElement("file");
    QDomAttr attr_source = doc.createAttribute("source");

    attr_source.setValue(mSourceFilePath);
    root.setAttributeNode(attr_source);

    for(int i = 0; i < mFragmentList.length(); i++){
        QDomElement fragment = doc.createElement("fragment");
        QDomAttr attr_id = doc.createAttribute("id");
        QDomElement text = doc.createElement("text");
        QDomElement trans = doc.createElement("trans");
        QDomElement words = doc.createElement("words");

        attr_id.setValue(QString::number(i));
        text.appendChild(doc.createTextNode(mFragmentList[i]));
        if(mFragmentTransList[i] != ""){
            trans.appendChild(doc.createTextNode(mFragmentTransList[i]));
            fragment.appendChild(trans);
        }
        words.appendChild(doc.createTextNode(conjFragmentWords(i)));
        fragment.appendChild(text);
        fragment.appendChild(words);
        fragment.setAttributeNode(attr_id);
        root.appendChild(fragment);
    }
    doc.appendChild(root);

    QString path = mSourceFilePath + ".wmtmp";
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "failed to open file for writing fragments";
        return;
    }
    QTextStream out(&file);
    doc.save(out, 4);
    file.close();
}

QString FragmentManager::conjFragmentWords(int index){
    QString result = "";
    int len = mFragmentWordList[index].length();
    for(int i = 0; i < len - 1; i++){
        result += mFragmentWordList[index][i] + "/#/";
    }
    if(len -1 >= 0){
        result += mFragmentWordList[index][len - 1];
    }
    return result;
}

QStringList FragmentManager::cutWords(const QString& content){
    if(mJieba == NULL){
        mJieba = new cppjieba::Jieba(JiebaPaths::jieba_dict,
                                    JiebaPaths::hmm_model,
                                    JiebaPaths::user_dict,
                                    JiebaPaths::idf,
                                    JiebaPaths::stop_words);
    }

    std::vector<std::string> words;
    std::string text = content.toStdString();

    mJieba->Cut(text, words, true);

    QStringList result;
    for(int i = 0; i < words.size(); i++){
        QString word = QString::fromStdString(words[i]);
        if(!word.isEmpty()){
            result.append(word);
        }
    }
    return result;
}

void FragmentManager::reloadJiebaDict(){
    if(mJieba != NULL){
        delete mJieba;
        mJieba = NULL;
    }
    mJieba = new cppjieba::Jieba(JiebaPaths::jieba_dict,
                                 JiebaPaths::hmm_model,
                                 JiebaPaths::user_dict,
                                 JiebaPaths::idf,
                                 JiebaPaths::stop_words);
}

void FragmentManager::rebuildCurrentFragment(){
    mFragmentWordList[mCurrentIndex] = cutWords(mFragmentList[mCurrentIndex]);
    flushRecords();
}
