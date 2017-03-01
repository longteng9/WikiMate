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

namespace cppjieba{
    class Jieba;
}

struct JiebaPaths{
    static const char* jieba_dict;
    static const char* user_dict;
    static const char* hmm_model;
    static const char* idf;
    static const char* stop_words;
};

class FragmentManager : public QObject
{
    Q_OBJECT
    class GC{
    public:
        ~GC(){
            if(mInstance != NULL){
                delete mInstance;
                mInstance = NULL;
            }
        }
    };
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
    void reloadJiebaDict();
    void rebuildCurrentFragment();
    void exportTrans();

signals:

public slots:

private:
    explicit FragmentManager(QObject *parent = 0);
    FragmentManager(const FragmentManager&) = default;
    virtual ~FragmentManager();
    FragmentManager& operator=(const FragmentManager&) = default;

    QString conjFragmentWords(int index);
    QStringList cutWords(const QString& content);

public:
    QVector<QStringList> mFragmentWordList;
    QStringList mFragmentList;
    QStringList mFragmentTransList;
    int mCurrentIndex = 0;
    QString mSourceFilePath = "";

private:
    static FragmentManager *mInstance;
    static GC gc;
    cppjieba::Jieba *mJieba = NULL;
};


#endif // FRAGMENTMANAGER_H
