#include "FileItemDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QPixmap>

FileItemDelegate::FileItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}


void FileItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if(index.column() == 0) {
        painter->save();
        QPixmap pic(index.model()->data(index).toString());
        if(pic.isNull()){
            pic = QPixmap(":/static/file-icon-green.jpg");
        }
        pic = pic.scaled(option.rect.height() - 4, option.rect.height() - 4,  Qt::KeepAspectRatioByExpanding);
        int width = pic.width();
        int height = pic.height();
        QRect rect = option.rect;
        int x = rect.x() + rect.width()/2 - width/2;
        int y = rect.y() + rect.height()/2 - height/2;
        painter->drawPixmap(x, y, pic);
        painter->restore();
    } else if (index.column() == 5) {
        QString check = index.model()->data(index).toString();
        if (check.contains(' ')){
            return QItemDelegate::paint(painter, option, index);
        }
        int value = index.model()->data(index).toInt();
        QStyleOptionProgressBarV2 progressBarOption;
        progressBarOption.rect = option.rect.adjusted(4, 4, -4, -4);
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignRight;
        progressBarOption.textVisible = true;
        progressBarOption.progress = value;
        progressBarOption.text = tr("%1%").arg(progressBarOption.progress);

        painter->save();
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setBrush(option.palette.highlightedText());
        }
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
        painter->restore();
    } else {
        return QItemDelegate::paint(painter, option, index);
    }
}
