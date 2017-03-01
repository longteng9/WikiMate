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
#include "DictEngine.h"

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
        showTransMemTable();
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
            mMessageForm->setTitle("Processing source file, just a moment... ");
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
    mOriginSelection = ui->txtOriginal->textCursor().selectedText();
}

void MainWindow::on_btnNextFrag_clicked()
{
    if(!ui->btnNextFrag->isEnabled()){
        qDebug() << "please wait until entry table initialized";
        ui->statusLabel->setText("Please wait until the entry table is initialized.");
        return;
    }
    ui->btnNextFrag->setEnabled(false);
    DictEngine::instance()->stopQueryAndFetch();
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

void MainWindow::on_btnToggleOD_clicked()
{
    mEnableOnlineDict = !mEnableOnlineDict;
    if(mEnableOnlineDict){
        ui->btnToggleOD->setText("Disable OD");
    }else{
        ui->btnToggleOD->setText("Enable OD");
    }
}

void MainWindow::on_btnCommitTMs_clicked()
{
    FragmentManager::instance()->reloadJiebaDict();
    FragmentManager::instance()->rebuildCurrentFragment();
    showEntriesTableAsync(FragmentManager::instance()->currentFragmentWords());
}

void MainWindow::on_tableEntries_itemChanged(QTableWidgetItem *item)
{
    if(ui->btnNextFrag->isEnabled()){
        QString word = ui->tableEntries->horizontalHeaderItem(item->column())->text();
        qDebug() << word<< ": "<< item->text();
        DictEngine::instance()->insertTransMem(word, item->text());
    }
}

void MainWindow::on_editKeyword_returnPressed()
{
    QMap<QString, QString> result = DictEngine::instance()->queryWikiDumpEntry(ui->editKeyword->text());
    if(result.isEmpty()){
        ui->lstIndex->insertItem(0, "No Wiki Entry");
        return;
    }
    QListWidgetItem * item = new QListWidgetItem;
    item->setSizeHint(QSize(60, 28));  //每次改变Item的高度
    item->setText(ui->editKeyword->text() + "/" + result["page_id"]);
    ui->lstIndex->addItem(item);
    ui->lstIndex->setFocus();
}

void MainWindow::on_lstIndex_itemPressed(QListWidgetItem *item)
{
    QStringList tmp = item->text().split("/", QString::KeepEmptyParts);
    if(tmp.size() > 1){
        QString page_id = tmp[tmp.size()-1];
        QString wikiPage = DictEngine::instance()->queryWikiPageById(page_id);
        if(wikiPage != ""){
            ui->txtWikiPage->setText(wikiPage);
            ui->tabWidget->setCurrentIndex(1);
        }
    }
}

void MainWindow::on_lstIndex_itemClicked(QListWidgetItem *item)
{
    on_lstIndex_itemPressed(item);
}

void MainWindow::on_btnExport_clicked()
{
    FragmentManager::instance()->exportTrans();
}
