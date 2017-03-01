#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QListWidgetItem>
#include <QColor>
#include <QFileDialog>
#include "Helper.h"
#include <QMenu>
#include "FragmentManager.h"
#include "FileTableModel.h"
#include "FileTableView.h"

void MainWindow::on_lstMenu_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous){
        previous->setBackground(Qt::white);
    }
    current->setBackgroundColor(QColor("#0F7DBE"));
    if(current->text() == "Tasks"){
        ui->tabCenter->setCurrentIndex(0);
        ui->tabLeftSide->setCurrentIndex(0);
        ui->tabTopTools->setCurrentIndex(0);
    }else if(current->text() == "Editor"){
        ui->tabCenter->setCurrentIndex(1);
        ui->tabLeftSide->setCurrentIndex(1);
        ui->tabTopTools->setCurrentIndex(1);
        ui->tabWidget->setCurrentIndex(0);
    }else if(current->text() == "Trans-Mem"){
        ui->tabCenter->setCurrentIndex(2);
        ui->tabLeftSide->setCurrentIndex(2);
        ui->tabTopTools->setCurrentIndex(2);
    }
}

void MainWindow::on_lstTaskFilter_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous){
        previous->setBackground(Qt::white);
    }
    current->setBackgroundColor(QColor("#0F7DBE"));
}

void MainWindow::on_btnWorkingDir_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Choose Working Directory", "./");
    if(!dirPath.isEmpty()){
        Helper::instance()->mWorkingStatusQuo["projectDirectory"] = dirPath;
        Helper::instance()->mProjectDirectory = dirPath;
        Helper::instance()->refreshWorkingDir(dirPath);
        this->setWindowTitle("WikiMate @ " + Helper::instance()->mProjectDirectory);
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(dirPath);
        updateFileList(files);
    }
}

void MainWindow::on_lstTaskFilter_currentTextChanged(const QString &currentText)
{
    if(currentText == "All Tasks"){
        filterFileList("");
    }else if(currentText == "Tasks [DOING]"){
        filterFileList("DOING");
    }else if(currentText == "Tasks [DONE]"){
        filterFileList("DONE");
    }
}

void MainWindow::on_btnRefreshTasks_clicked()
{
    qDebug() << "refreshing list";
    Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
    QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
    updateFileList(files);
}

void MainWindow::on_btnStartTask_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        ui->statusLabel->setText("Building fragment map, please wait...");
        this->mMessageForm->show();
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mProjectDirectory, fileName);
        qDebug() << "start task:" << path;

        if(FragmentManager::instance()->mSourceFilePath != path){
            FragmentManager::instance()->mSourceFilePath = path;
            qDebug() << "start async build fragments";
            ui->lstMenu->setCurrentRow(1);
            ui->statusLabel->setText("Working on <strong>" +
                                     FragmentManager::instance()->mSourceFilePath.mid(
                                         FragmentManager::instance()->mSourceFilePath.lastIndexOf("/")+1) + "</strong>");


            AsyncBuildFragment *worker = new AsyncBuildFragment;
            connect(worker, &AsyncBuildFragment::finished, worker, &AsyncBuildFragment::deleteLater, Qt::QueuedConnection);
            connect(worker, &AsyncBuildFragment::finished, this, &MainWindow::on_buildFragmentFinished, Qt::QueuedConnection);
            mLauncher->asyncRun(worker, "start");
            mMessageForm->setText("Processing source file, just a moment... ");
            mMessageForm->show();
        }
    }
}

void MainWindow::on_btnExportTask_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mProjectDirectory, fileName);
        qDebug() << path;
    }
}

void MainWindow::on_btnAddTasks_clicked()
{
    Helper::instance()->addNewTasks(this);
    Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
    QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
    updateFileList(files);
}

void MainWindow::on_btnRemoveTasks_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mProjectDirectory, fileName);
        QFile::remove(path);
        Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
        updateFileList(files);
    }
}

void MainWindow::on_txtOriginal_cursorPositionChanged()
{
}

void MainWindow::on_txtOriginal_selectionChanged()
{
    QString selected = ui->txtOriginal->textCursor().selectedText();
    if(selected.isEmpty()){
        return;
    }

    qDebug() << "select:" << selected;
    qDebug() << FragmentManager::instance()->retrieveWord(selected);
}


void MainWindow::on_btnSaveTransMem_clicked()
{
    QStringList headers;
    QStringList rows;
    QStringList prev_list;

}

void MainWindow::on_btnNextFrag_clicked()
{
    QString trans = ui->txtTrans->toPlainText();
    if(trans != ""){
        FragmentManager::instance()->updateFragmentTrans(ui->txtTrans->toPlainText());
    }
    setCurrentFragment(FragmentManager::instance()->mCurrentIndex + 1);
}

void MainWindow::on_btnPrevFrag_clicked()
{
    QString trans = ui->txtTrans->toPlainText();
    if(trans != ""){
        FragmentManager::instance()->updateFragmentTrans(ui->txtTrans->toPlainText());
    }
    setCurrentFragment(FragmentManager::instance()->mCurrentIndex - 1);
}

void MainWindow::on_btnSaveFrag_clicked()
{
    FragmentManager::instance()->updateFragmentTrans(ui->txtTrans->toPlainText());
}

void MainWindow::on_btnCommitTMs_clicked()
{
    FragmentManager::instance()->reloadJiebaDict();
    FragmentManager::instance()->rebuildCurrentFragment();
    showEntriesTableAsync(FragmentManager::instance()->currentFragmentWords());
}

void MainWindow::on_txtOriginal_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = ui->txtOriginal->createStandardContextMenu();
    menu->addAction("Add Trans-Mem");
    menu->exec(pos);
    connect(menu, &QMenu::triggered, [this](QAction* action){
        qDebug() << "clicked on:" << action->text();
    });
}

void MainWindow::on_tableEntries_itemChanged(QTableWidgetItem *item)
{

}

