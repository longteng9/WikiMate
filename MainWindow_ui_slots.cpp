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
#include <QScrollBar>
#include <QDesktopServices>

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
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mProjectDirectory, fileName);
        if(!FragmentManager::instance()->jiebaValid()){
            qDebug() << "JiebaPath is invalid, quit starting task: " << path;
            return;
        }

        QFileInfo info(path);
        if(!info.isFile() || !info.exists()){
            return;
        }

        if(FragmentManager::instance()->mSourceFilePath != path){
            qDebug() << "start task:" << path;
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

            MessageForm::createAndShowAs(MessageForm::Role::LoadingForm, this);
        }
    }
}

void MainWindow::on_btnExportTask_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mProjectDirectory, fileName);
        QFileInfo info(path);
        if(!info.isFile() || !info.exists()){
            return;
        }
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
    QString trans = ui->txtTrans->toPlainText();
    if(trans != ""){
        FragmentManager::instance()->updateFragmentTrans(ui->txtTrans->toPlainText());
    }
    setCurrentFragment(FragmentManager::instance()->mCurrentIndex + 1);

    autoScrollOriginBrowser(FragmentManager::instance()->mCurrentIndex);
}

void MainWindow::on_btnPrevFrag_clicked()
{
    QString trans = ui->txtTrans->toPlainText();
    if(trans != ""){
        FragmentManager::instance()->updateFragmentTrans(ui->txtTrans->toPlainText());
    }
    setCurrentFragment(FragmentManager::instance()->mCurrentIndex - 1);

    autoScrollOriginBrowser(FragmentManager::instance()->mCurrentIndex);
}

void MainWindow::on_btnSaveFrag_clicked()
{
    FragmentManager::instance()->updateFragmentTrans(ui->txtTrans->toPlainText());
}

void MainWindow::on_btnToggleOD_clicked()
{
    mEnableOnlineDict = false;
    /*
    mEnableOnlineDict = !mEnableOnlineDict;
    if(mEnableOnlineDict){
        ui->btnToggleOD->setText("Disable OD");
    }else{
        DictEngine::instance()->stopQueryAndFetch();
        ui->btnPrevFrag->setEnabled(true);
        ui->btnNextFrag->setEnabled(true);
        ui->btnToggleOD->setText("Enable OD");
    }
    */
}

void MainWindow::on_btnRefreshEntries_clicked()
{
    FragmentManager::instance()->reloadJiebaDict();
    FragmentManager::instance()->rebuildCurrentFragment();
    showEntriesTableAsync(FragmentManager::instance()->currentFragmentWords());
}

void MainWindow::on_tableEntries_itemChanged(QTableWidgetItem *item)
{
    if(ui->btnNextFrag->isEnabled() && ui->tableEntries->editTriggers() != QAbstractItemView::NoEditTriggers){
        QString word = ui->tableEntries->horizontalHeaderItem(item->column())->text();
        qDebug() << "insert trans-mem " << word<< ": "<< item->text();
        DictEngine::instance()->insertTransMem(word, item->text());
    }
}

void MainWindow::on_editKeyword_returnPressed()
{
    on_editKeyword_textChanged(ui->editKeyword->text());
}

void MainWindow::on_editKeyword_textChanged(const QString &arg1)
{
    if(arg1.isEmpty()){
        return;
    }
    ui->lstIndex->clear();
    QVector<QMap<QString, QString> > result = DictEngine::instance()->queryWikiDumpEntryFuzzy(arg1);
    if(result.isEmpty()){
        ui->lstIndex->insertItem(0, "No Wiki Entry");
        return;
    }


    for(int i = 0; i < result.length(); i++){
        QListWidgetItem * item = new QListWidgetItem;
        item->setSizeHint(QSize(60, 30));  //每次改变Item的高度
        item->setText(result[i]["page_title"] + "/" + result[i]["page_id"]);
        ui->lstIndex->addItem(item);
    }
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
    QString content = FragmentManager::instance()->getExportContent();
    ui->txtExportPreview->setText(content);
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::on_btnWikiOnline_clicked()
{
    QDesktopServices::openUrl(QUrl("https://en.wiktionary.org/"));
}

void MainWindow::on_btnEraseTransMem_clicked()
{
    DictEngine::instance()->eraseTransMemAll();
}

void MainWindow::on_btnEntryTableEditToggle_clicked()
{
    if(ui->tableEntries->editTriggers() != QAbstractItemView::NoEditTriggers){
        ui->tableEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->btnEntryTableEditToggle->setText("Enable Edit Entries");
    }else{
        ui->tableEntries->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        ui->btnEntryTableEditToggle->setText("Disable Edit Entries");
    }
}

void MainWindow::on_btnConfirmExport_clicked()
{
    QString confirmedContent = ui->txtExportPreview->toPlainText();

    QStringList tmp = FragmentManager::instance()->mSourceFilePath.replace("\\", "/").split("/");
    tmp[tmp.length() - 1] = "[EN]" + tmp[tmp.length()-1];
    QString newPath = "";
    for(int i = 0; i < tmp.size(); i++){
        newPath += tmp[i] + "/";

    }
    newPath = newPath.mid(0, newPath.length() - 1);

    QFile file(newPath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "failed to open file: " << newPath;
        return;
    }
    file.write(confirmedContent.toUtf8());
    file.close();
    qDebug() << "exported task: " << FragmentManager::instance()->mSourceFilePath;
    qDebug() << "to dest path: " << newPath;
    ui->statusLabel->setText(QString("succeed to export: ") + newPath);
}

void MainWindow::on_tableEntries_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if(current != NULL){
        QString entryContent = current->text();
        ui->txtEntryEnlargeView->setText(entryContent);
    }
}
