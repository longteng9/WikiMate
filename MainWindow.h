#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

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
    void on_update_fileList(const QList<QMap<QString, QString> > & data);

private slots:
    void on_lstMenu_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lstFileOptions_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void initUI();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
