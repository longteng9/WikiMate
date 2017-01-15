#include "FileTableModel.h"

FileTableModel::FileTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

FileTableModel::~FileTableModel()
{

}


int FileTableModel::rowCount(const QModelIndex &parent) const
{
    return mData.size();
}

int FileTableModel::columnCount(const QModelIndex &parent) const
{
    return mHorizontalHeader.count();
}

QVariant FileTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole) {
        int ncol = index.column();
        int nrow =  index.row();
        QStringList values = mData.at(nrow);
        if (values.size() > ncol)
            return values.at(ncol);
        else
        return QVariant();
    }
    return QVariant();
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    // flag |= Qt::ItemIsEditable // 设置单元格可编辑, 此处注释,单元格无法被编辑
    return flag;
}

void FileTableModel::setHorizontalHeader(const QStringList &headers)
{
    mHorizontalHeader =  headers;
}


QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return mHorizontalHeader.at(section);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void FileTableModel::setData(const QVector<QStringList> &data)
{
    mData = data;
}
