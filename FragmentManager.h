#ifndef FRAGMENTMANAGER_H
#define FRAGMENTMANAGER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QMessageBox>

class FragmentManager : public QObject
{
    Q_OBJECT
public:
    static FragmentManager* instance();
    QVector<QStringList> buildFragments(const QStringList &sentences);
    QString retrieveFragment(int block, int pos);
    bool retrieveWord(QString word);

signals:

public slots:

private:
    explicit FragmentManager(QObject *parent = 0);
    FragmentManager(const FragmentManager&) = default;
    ~FragmentManager() = default;
    FragmentManager& operator=(const FragmentManager&) = default;

public:
    QVector<QStringList> mCurrentTaskBlocksFragments;

private:
    static FragmentManager *mInstance;
};

class PythonThread : public QThread{
    Q_OBJECT
  protected:
    void run();

public:
    QString mCmdIn = "python mandarin.py";
    QString mStdout = "";
    QString mStderr = "";
};


#endif // FRAGMENTMANAGER_H
