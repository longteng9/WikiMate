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
    void on_startTransEditing(const QString &path);

private slots:
    void on_lstMenu_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lstFileOptions_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_btnWorkingDir_clicked();

    void on_lstMenu_clicked(const QModelIndex &index);

private:
    void initUI();
    void updateFileList(const QVector<QStringList> &data);
    void restoreHistory();
    void saveHistory();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
