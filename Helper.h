#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QVector>

class Helper : public QObject
{
    Q_OBJECT
public:
    static Helper* instance();
    void scanFolder(const QString& path, QStringList *result, bool recur = true);
    QVector<QStringList> getWorkingFiles(const QString& dirPath);
    QString pathJoin(QString part1, QString part2);
    void refreshWorkingDir(QString dirPath);
    QString sciSize(int64_t size);
    void addNewTasks(QWidget* parent);
    bool copyFile(const QString& absPath, const QString& newPath, bool overwrite = false);
    QStringList readForSentences(const QString& path);

signals:
    void refreshTaskList();

public slots:

private:
    explicit Helper(QObject *parent = 0);
    Helper(const Helper&) = default;
    ~Helper() = default;
    Helper& operator=(const Helper&) = default;

public:
    QMap<QString, QString> mWorkingHistory;
    QString mCurrenttDirectory;
    QVector<QStringList> mTaskListBackup;
    QString mCurrentTaskPath;
    QVector<QStringList> mCurrentTaskBlockFragments;

private:
    static Helper *mInstance;
};

#endif // HELPER_H
