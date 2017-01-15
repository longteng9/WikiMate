#ifndef FILETABLEVIEW_H
#define FILETABLEVIEW_H

#include <QTableView>

class FileTableModel;
class FileItemDelegate;

class FileTableView : public QTableView
{
    Q_OBJECT
public:
    explicit FileTableView(QWidget *parent = 0);
    FileTableModel* tableModel() {return mModel;}
    ~FileTableView();

private:
    void initUI();
    void initData();

signals:

public slots:
    void act_openAndTrans();
    void act_closeAndFinalize();
    void act_addFiles();
    void act_deleteFile();
    void act_openFolder();

private slots:
    void onCreateContextMenu(const QPoint &point);

private:
    FileTableModel *mModel;
    FileItemDelegate *mItemDelegate;
    QMenu *mContextMenu;
};

#endif // FILETABLEVIEW_H
