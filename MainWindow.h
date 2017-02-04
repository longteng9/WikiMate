#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_btnRefreshTasks_clicked();
    void on_btnExportTask_clicked();
    void on_btnAddTasks_clicked();
    void on_btnRemoveTasks_clicked();

private slots:
    void on_lstMenu_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lstFileOptions_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_btnWorkingDir_clicked();

    void on_lstMenu_clicked(const QModelIndex &index);

    void on_lstTaskFilter_currentTextChanged(const QString &currentText);

    void on_btnStartTask_clicked();

    void on_txtOriginal_cursorPositionChanged();

    void on_txtOriginal_selectionChanged();

private:
    void initUI();
    void updateFileList(const QVector<QStringList> &data);
    void filterFileList(const QString& keyword);
    void restoreHistory();
    void saveHistory();


private:

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
