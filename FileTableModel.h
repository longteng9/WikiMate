#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H
#include <QAbstractTableModel>

class FileTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit FileTableModel(QObject *parent = 0);
    ~FileTableModel(void);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setHorizontalHeader(const QStringList& headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setData(const QVector<QStringList>& data);
    QVector<QStringList>& DataVector() {return mData;}

signals:

public slots:

private:
    QStringList mHorizontalHeader;
    QVector<QStringList> mData;
};

#endif // FILETABLEMODEL_H
