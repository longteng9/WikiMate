#ifndef FRAGMENTMANAGER_H
#define FRAGMENTMANAGER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QMessageBox>
#include <QProcess>

class FragmentManager : public QObject
{
    Q_OBJECT
public:
    static FragmentManager* instance();
    void buildFragments(const QString &path);
    bool retrieveWord(QString word);
    QStringList currentBlockFragments();

signals:

public slots:

private:
    explicit FragmentManager(QObject *parent = 0);
    FragmentManager(const FragmentManager&) = default;
    ~FragmentManager() = default;
    FragmentManager& operator=(const FragmentManager&) = default;

public:
    QVector<QStringList> mFragmentWordList;
    QStringList mFragmentList;
    QStringList mFragmentTransList;
    int mCurrentIndex = 0;

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
