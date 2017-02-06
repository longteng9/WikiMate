#ifndef FRAGMENTMANAGER_H
#define FRAGMENTMANAGER_H

#include <QObject>
#include <QVector>

class FragmentManager : public QObject
{
    Q_OBJECT
public:
    static FragmentManager* instance();
    QVector<QStringList> buildFragments(const QStringList &sentences);
    QString retrieveFragment(int block, int pos);


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

#endif // FRAGMENTMANAGER_H
