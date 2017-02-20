#ifndef FRAGMENTMANAGER_H
#define FRAGMENTMANAGER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QMessageBox>
#include <QProcess>

/*
记录文件的格式：
<file source="file/path">
    <fragment id="0">
        <text>我是中文句子</text>
        <trans>i am chinese sentence</trans> <!--如果没有对应的翻译则不存在这个元素-->
        <words>我/#/是/#/中文/#/句子</words>
    </fragment>
    <fragment id="1">
        <text>我是中文句子2</text>
        <trans>i am chinese sentence2</trans> <!--如果没有对应的翻译则不存在这个元素-->
        <words>我/#/是/#/中文/#/句子</words>
    </fragment>
</file>
*/
class FragmentManager : public QObject
{
    Q_OBJECT
public:
    static FragmentManager* instance();
    void flushRecords();
    void buildOrLoadFragments(const QString &path);
    void buildFragments(const QString &path);
    void loadRecords(const QString &path);
    bool retrieveWord(QString word);
    QStringList currentFragmentWords();
    QString getFormatContent();
    void updateFragmentTrans(const QString& trans);

signals:

public slots:

private:
    explicit FragmentManager(QObject *parent = 0);
    FragmentManager(const FragmentManager&) = default;
    ~FragmentManager() = default;
    FragmentManager& operator=(const FragmentManager&) = default;
    QString conjFragmentWords(int index);

public:
    QVector<QStringList> mFragmentWordList;
    QStringList mFragmentList;
    QStringList mFragmentTransList;
    int mCurrentIndex = 0;
    QString mSourceFilePath = "";

private:
    static FragmentManager *mInstance;
};

class PythonThread : public QThread{
    Q_OBJECT
protected:
    void run();

public:
    QString mStdout = "";
    QString mStderr = "";
    int code = 0;
};


#endif // FRAGMENTMANAGER_H
