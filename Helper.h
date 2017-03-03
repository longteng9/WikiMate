#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QVector>
#include <QMap>

class Helper : public QObject
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
    static Helper* instance();
    void scanFolder(const QString& path, QStringList *result, bool recur = true);
    QVector<QStringList> getWorkingFiles(const QString& dirPath);
    QString pathJoin(QString part1, QString part2);
    void refreshWorkingDir(QString dirPath);
    QString sciSize(int64_t size);
    void addNewTasks(QWidget* parent);
    bool copyFile(const QString& absPath, const QString& newPath, bool overwrite = false);
    bool isUTF8File(const QString& path);
    void updateProjectFile(const QString& taskname, const QString& attr, const QString& value);
    bool equalStringList(const QStringList& list1, const QStringList& list2);

signals:
    void refreshTaskList();

public slots:

private:
    explicit Helper(QObject *parent = 0);
    Helper(const Helper&) = default;
    ~Helper() = default;
    Helper& operator=(const Helper&) = default;

public:
    QMap<QString, QString> mWorkingStatusQuo;
    QString mProjectDirectory;
    QVector<QStringList> mTaskListBackup;

private:
    static Helper *mInstance;
    static GC gc;
};

#endif // HELPER_H
