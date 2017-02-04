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
    FileTableModel* tableModel() {
        return mModel;
    }
    void updateData(const QVector<QStringList> &data);
    ~FileTableView();

private:
    void initUI();
    void initData();

signals:
    void startTransEditing();
    void startExportTask();
    void startAddNewTasks();
    void startRemoveTasks();
    void refreshTaskList();

public slots:
    void act_startTask();
    void act_exportTask();
    void act_addTasks();
    void act_removeTasks();
    void act_openFolder();
    void act_refreshList();

private slots:
    void onCreateContextMenu(const QPoint &point);
    void onDoubleClicked(const QModelIndex &index);

private:
    FileTableModel *mModel;
    FileItemDelegate *mItemDelegate;
    QMenu *mContextMenu;
};

#endif // FILETABLEVIEW_H
