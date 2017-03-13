#include "DocxManager.h"
#include "Archive.h"
#include <QStandardPaths>
#include "Helper.h"
#include <QFile>
#include <QDebug>
#include <QDomDocument>

DocxManager::DocxManager(QObject *parent)
    : QObject(parent)
{

}

int DocxManager::getWordCount(const QString& path){
    zip::Archive archive;
    std::string dup = path.toStdString();
    dup = dup.substr(dup.find_last_of("\\/")+1);
    dup = dup.substr(0, dup.find_last_of("."));
    int err = archive.uncompress(path.toStdString(),
                                 Helper::instance()->pathJoin(QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace("\\", "/"), QString(dup.c_str())).toStdString());
    if(err != 0){
        qDebug() <<"failed to uncompress:" << path;
        return -1;
    }

    QString propPath = Helper::instance()->pathJoin(QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace("\\", "/"), QString(dup.c_str())) + "/docProps/app.xml";
    QFile file(propPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "failed to open: " << propPath;
        return -1;
    }
    QString content = file.readAll();
    int start = content.indexOf("<Words>");
    int end = content.indexOf("</Words>", start);
    QString strNum = content.mid(start + 7, end - (start + 7));
    return strNum.toInt();
}

QStringList DocxManager::getFragmentList(const QString& path, QMap<int, int> *fragParaMap){
    QStringList results;
    zip::Archive archive;
    std::string dup = path.toStdString();
    dup = dup.substr(dup.find_last_of("\\/")+1);
    dup = dup.substr(0, dup.find_last_of("."));
    int err = archive.uncompress(path.toStdString(),
                                 Helper::instance()->pathJoin(QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace("\\", "/"), QString(dup.c_str())).toStdString());
    if(err != 0){
        qDebug() <<"failed to uncompress:" << path;
        return results;
    }

    QString documentPath = Helper::instance()->pathJoin(QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace("\\", "/"), QString(dup.c_str())) + "/word/document.xml";

    QString content = getDocument(documentPath);
    // 构建mFragmentList
    QStringList paragraphList = content.split("\n", QString::SkipEmptyParts);
    for(int paragraphId = 0; paragraphId < paragraphList.length(); paragraphId++){
        QString paragraph = paragraphList[paragraphId];
        int pre = 0;
        int pos = paragraph.indexOf(QRegExp("\u3002|\uff01|\uff1f|,|\uff0c"), pre);
        while (pos >= 0){
            QString frag = paragraph.mid(pre, pos - pre + 1).trimmed();
            // frag = frag.replace("\n", "");
            results.append(frag);
            fragParaMap->insert(results.length() - 1, paragraphId);
            pre = pos + 1;
            pos = paragraph.indexOf(QRegExp("\u3002|\uff01|\uff1f|,|\uff0c"), pre);
        }
        if(!paragraph.endsWith("\u3002") // 。
                && !paragraph.endsWith("\uff01") //！
                && !paragraph.endsWith("\uff1f")
                && !paragraph.endsWith("\uff0c")
                && !paragraph.endsWith(",")){ // ？
            results.append(paragraph.mid(pre));
            fragParaMap->insert(results.length() - 1, paragraphId);
        }
    }

    return results;
}

QString DocxManager::getDocument(const QString& path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        qDebug() << ".docx document";
        return "";
    }
    QString error;
    int errorLine;
    int errorCol;

    QDomDocument doc;
    if(!doc.setContent(&file, false, &error, &errorLine, &errorCol)){
        qDebug() << "failed to set content to QDomDocument instance";
        file.close();
        return "";
    }

    QString content = "";
    QDomElement root = doc.documentElement();
    QDomNode body = root.firstChild();
    QDomNode child = body.firstChild();
    while(!child.isNull()){
        if(child.isElement()){
           QString elem = child.toElement().text();
           content += elem + "\n";
        }
        child = child.nextSibling();
    }

    return content;
}
