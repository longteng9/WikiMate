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
#include <QDir>
#include <QCoreApplication>

QString JiebaPaths::jieba_dict = "";
QString JiebaPaths::user_dict = "";
QString JiebaPaths::hmm_model = "";
QString JiebaPaths::idf = "";
QString JiebaPaths::stop_words = "";

bool JiebaPaths::allFileValid(){
    QFileInfo file_dict(JiebaPaths::jieba_dict);
    if(!file_dict.exists()){
        return false;
    }
    QFileInfo file_user(JiebaPaths::user_dict);
    if(!file_user.exists()){
        return false;
    }
    QFileInfo file_hmm(JiebaPaths::hmm_model);
    if(!file_hmm.exists()){
        return false;
    }
    QFileInfo file_idf(JiebaPaths::idf);
    if(!file_idf.exists()){
        return false;
    }
    QFileInfo file_stops(JiebaPaths::stop_words);
    if(!file_stops.exists()){
        return false;
    }
    return true;
}

FragmentManager *FragmentManager::mInstance = NULL;
FragmentManager::GC FragmentManager::gc;

FragmentManager::FragmentManager(QObject *parent) : QObject(parent){
    JiebaPaths::jieba_dict = QCoreApplication::applicationDirPath() + "/dict/jieba/jieba.dict.utf8";
    JiebaPaths::user_dict = QCoreApplication::applicationDirPath() + "/dict/jieba/user.dict.utf8";
    JiebaPaths::hmm_model = QCoreApplication::applicationDirPath() + "/dict/jieba/hmm_model.utf8";
    JiebaPaths::idf = QCoreApplication::applicationDirPath() + "/dict/jieba/idf.utf8";
    JiebaPaths::stop_words = QCoreApplication::applicationDirPath() + "/dict/jieba/stop_words.utf8";

    if(!JiebaPaths::allFileValid()){
        qDebug() << "JibaPath is invalid:";
        qDebug() << JiebaPaths::jieba_dict
                 << "\n" << JiebaPaths::user_dict
                 << "\n" << JiebaPaths::hmm_model
                 << "\n" << JiebaPaths::idf
                 << "\n" << JiebaPaths::stop_words;
        MessageForm::createAndShowAs(MessageForm::Role::QueryDialogForm,
                                     "Warning",
                                     "Data files for Jieba component are missing or invalid, contact Xavier to get the data files and restart program.",
                                     [](bool val){
            qApp->quit();
        });
        mJieba = NULL;
    }else{
        mJieba = new cppjieba::Jieba(JiebaPaths::jieba_dict.toStdString().c_str(),
                                     JiebaPaths::hmm_model.toStdString().c_str(),
                                     JiebaPaths::user_dict.toStdString().c_str(),
                                     JiebaPaths::idf.toStdString().c_str(),
                                     JiebaPaths::stop_words.toStdString().c_str());
    }
}

bool FragmentManager::jiebaValid(){
    return mJieba != NULL;
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
    if(mJieba == NULL){
        return;
    }
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
    if(mJieba == NULL){
        return;
    }
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

    // 构建mFragmentList

    QStringList paragraphList = content.split("\n", QString::SkipEmptyParts);
    for(int paragraphId = 0; paragraphId < paragraphList.length(); paragraphId++){
        QString paragraph = paragraphList[paragraphId];
        int pre = 0;
        int pos = paragraph.indexOf(QRegExp("\u3002|\uff01|\uff1f"), pre);
        while (pos >= 0){
            QString frag = paragraph.mid(pre, pos - pre + 1).trimmed();
            // frag = frag.replace("\n", "");
            mFragmentList.append(frag);
            mFragmentIdToParagraphIdMap[mFragmentList.length() - 1] = paragraphId;
            pre = pos + 1;
            pos = paragraph.indexOf(QRegExp("\u3002|\uff01|\uff1f"), pre);
        }
        if(!paragraph.endsWith("\u3002") // 。
                && !paragraph.endsWith("\uff01") //！
                && !paragraph.endsWith("\uff1f")){ // ？
            mFragmentList.append(paragraph.mid(pre));
            mFragmentIdToParagraphIdMap[mFragmentList.length() - 1] = paragraphId;
        }
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
    if(mJieba == NULL){
        false;
    }
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
    if(mJieba == NULL){
        return QStringList();
    }
    return mFragmentWordList[mCurrentIndex];
}

QString FragmentManager::getFormatContent(){
    if(mJieba == NULL){
        return "";
    }
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
    if(mJieba == NULL || mCurrentIndex >= mFragmentTransList.length()){
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
    if(mJieba == NULL){
        return;
    }
    mFragmentList.clear();
    mFragmentTransList.clear();
    mFragmentWordList.clear();
    mFragmentIdToParagraphIdMap.clear();
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        qDebug() << "failed to open .wmtmp file";
        buildFragments(path.mid(0, path.length() - 6));
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
            int paragraphId = fragment.attribute("paragraphId").toInt();
            mFragmentIdToParagraphIdMap[id] = paragraphId;

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
    if(mJieba == NULL){
        return;
    }
    QDomDocument doc;
    QDomElement root = doc.createElement("file");
    QDomAttr attr_source = doc.createAttribute("source");

    attr_source.setValue(mSourceFilePath);
    root.setAttributeNode(attr_source);

    for(int i = 0; i < mFragmentList.length(); i++){
        QDomElement fragment = doc.createElement("fragment");
        QDomAttr attr_id = doc.createAttribute("id");
        QDomAttr attr_paragraphId = doc.createAttribute("paragraphId");
        QDomElement text = doc.createElement("text");
        QDomElement trans = doc.createElement("trans");
        QDomElement words = doc.createElement("words");

        attr_id.setValue(QString::number(i));
        attr_paragraphId.setValue(QString::number(mFragmentIdToParagraphIdMap[i]));
        text.appendChild(doc.createTextNode(mFragmentList[i]));
        if(mFragmentTransList[i] != ""){
            trans.appendChild(doc.createTextNode(mFragmentTransList[i]));
            fragment.appendChild(trans);
        }
        words.appendChild(doc.createTextNode(conjFragmentWords(i)));
        fragment.appendChild(text);
        fragment.appendChild(words);
        fragment.setAttributeNode(attr_id);
        fragment.setAttributeNode(attr_paragraphId);
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
    if(mJieba == NULL){
        return "";
    }
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
        return QStringList();
    }
    if(mJieba == NULL){
        mJieba = new cppjieba::Jieba(JiebaPaths::jieba_dict.toStdString().c_str(),
                                    JiebaPaths::hmm_model.toStdString().c_str(),
                                    JiebaPaths::user_dict.toStdString().c_str(),
                                    JiebaPaths::idf.toStdString().c_str(),
                                    JiebaPaths::stop_words.toStdString().c_str());
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
    if(mJieba == NULL){
        return;
    }
    delete mJieba;
    mJieba = new cppjieba::Jieba(JiebaPaths::jieba_dict.toStdString().c_str(),
                                 JiebaPaths::hmm_model.toStdString().c_str(),
                                 JiebaPaths::user_dict.toStdString().c_str(),
                                 JiebaPaths::idf.toStdString().c_str(),
                                 JiebaPaths::stop_words.toStdString().c_str());
}

void FragmentManager::rebuildCurrentFragment(){
    if(mJieba == NULL){
        return;
    }
    mFragmentWordList[mCurrentIndex] = cutWords(mFragmentList[mCurrentIndex]);
    flushRecords();
}

QString FragmentManager::getExportContent(){
    if(mJieba == NULL){
        return "";
    }
    QString content = "";

    int prevParagraphId = 0;
    for(int i = 0; i < mFragmentTransList.length(); i++){
        if(mFragmentTransList[i].isEmpty()){
            content += mFragmentList[i];
        }else{
            content += mFragmentTransList[i];
        }
        if(mFragmentIdToParagraphIdMap.size() <= i
                || mFragmentIdToParagraphIdMap[i] > prevParagraphId){
            content += "\n";
        }
    }

    return content;
}
