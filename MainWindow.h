#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMap>
#include <QRunnable>
#include <QThread>

namespace Ui {
class MainWindow;
}

class AsyncBuildFragment : public QThread{
    Q_OBJECT
private:
    void run();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:


public slots:
    void on_btnRefreshTasks_clicked();
    void on_btnExportTask_clicked();
    void on_btnAddTasks_clicked();
    void on_btnRemoveTasks_clicked();
    void on_btnSaveTransMem_clicked();
    void on_btnNextFrag_clicked();
    void on_btnPrevFrag_clicked();
    void on_btnSaveFrag_clicked();
    void on_fragmentDataReady();

private slots:
    void on_lstMenu_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lstTaskFilter_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_btnWorkingDir_clicked();

    void on_lstTaskFilter_currentTextChanged(const QString &currentText);

    void on_btnStartTask_clicked();

    void on_txtOriginal_cursorPositionChanged();

    void on_txtOriginal_selectionChanged();

    void on_btnCommitTMs_clicked();

    void on_txtOriginal_customContextMenuRequested(const QPoint &pos);

private:
    void initUI();
    void initFilter();
    void updateFileList(const QVector<QStringList> &data);
    void filterFileList(const QString& keyword);
    void restoreHistory();
    void saveHistory();
    void showEntriesTable(const QStringList &header);
    void setCurrentFragment(int index);


public:
    static MainWindow *windowRef;
    Ui::MainWindow *ui;

private:
    AsyncBuildFragment mAsyncBuildFragment;
};

class EntryTableKeyFilter : public QObject{
    Q_OBJECT
public:
    EntryTableKeyFilter(QObject *parent = 0){}
    bool eventFilter(QObject *watched, QEvent *event);
};

class FragmentEditorKeyFilter : public QObject{
    Q_OBJECT
public:
    FragmentEditorKeyFilter(QObject *parent = 0){}
    bool eventFilter(QObject *watched, QEvent *event);
};


#endif // MAINWINDOW_H
