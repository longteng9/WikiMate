#ifndef DOCXMANAGER_H
#define DOCXMANAGER_H

#include <QObject>

class DocxManager : public QObject
{
    Q_OBJECT
public:
    explicit DocxManager(QObject *parent = 0);
    int getWordCount(const QString& path);
    QStringList getFragmentList(const QString& path, QMap<int, int>* fragParaMap);

protected:
    QString getDocument(const QString& path);

};

#endif // DOCXMANAGER_H
