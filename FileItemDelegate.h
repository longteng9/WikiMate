#ifndef FILEPROGRESSDELEGATE_H
#define FILEPROGRESSDELEGATE_H

#include <QObject>
#include <QItemDelegate>

class FileItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit FileItemDelegate(QObject *parent = 0);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

signals:

public slots:
};

#endif // FILEPROGRESSDELEGATE_H
