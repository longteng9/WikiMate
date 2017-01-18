#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include <QMap>

class Helper : public QObject
{
    Q_OBJECT
public:
    static Helper* instance();
    void scanFolder(const QString& path, QStringList *result, bool recur = true);
    QVector<QStringList> getWorkingFiles(const QString& path);
    QString pathJoin(QString part1, QString part2);

signals:

public slots:

private:
    explicit Helper(QObject *parent = 0);
    Helper(const Helper&) = default;
    ~Helper() = default;
    Helper& operator=(const Helper&) = default;

public:
    QMap<QString, QString> mWorkingHistory;
    QString mCurrenttDirectory;

private:
    static Helper *mInstance;
};

#endif // HELPER_H
