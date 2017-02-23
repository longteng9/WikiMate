#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include <QStandardItemModel>
#include "FileTableModel.h"
#include "Helper.h"
#include <QFileDialog>
#include <QScrollBar>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTextBlock>
#include <QModelIndex>
#include "FragmentManager.h"
#include <QEvent>
#include "TransMemory.h"
#include "DictEngine.h"
#include <QThreadPool>

MainWindow* MainWindow::windowRef = NULL;

bool EntryTableKeyFilter::eventFilter(QObject *watched, QEvent *event){
    int key;
    switch(event->type())  {
        case QEvent::KeyPress:
            key = (static_cast<QKeyEvent *>(event))->key();
            if (key == Qt::Key_I)  {
                QTableWidget* entryTable = MainWindow::windowRef->ui->tableEntries;
                entryTable->editItem(entryTable->item(entryTable->currentRow(), entryTable->currentColumn()));
                return true;
            }else if(key == Qt::Key_Enter
                     || key == Qt::Key_Return){
                QTableWidget* entryTable = MainWindow::windowRef->ui->tableEntries;
                if(!entryTable->item(entryTable->currentRow(), entryTable->currentColumn())){
                    return false;
                }
                QString entry = entryTable->item(entryTable->currentRow(), entryTable->currentColumn())->text();
                MainWindow::windowRef->ui->txtTrans->setText(MainWindow::windowRef->ui->txtTrans->toPlainText() + entry + " ");
                return true;
            }else if(key == Qt::Key_Down
                     && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::AltModifier)){
                MainWindow::windowRef->ui->txtTrans->setFocus();
                QTextCursor cursor = MainWindow::windowRef->ui->txtTrans->textCursor();
                cursor.movePosition(QTextCursor::MoveOperation::EndOfBlock);
                MainWindow::windowRef->ui->txtTrans->setTextCursor(cursor);
                return true;
            }else if(key == Qt::Key_S
                     && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                MainWindow::windowRef->on_btnSaveTransMem_clicked();
                return true;
            }
        default:
            return false;
    }
    return false;
}

bool FragmentEditorKeyFilter::eventFilter(QObject *watched, QEvent *event){
    int key;
    switch(event->type())  {
        case QEvent::KeyPress:
            key = (static_cast<QKeyEvent *>(event))->key();
            if(key == Qt::Key_Up
                     && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::AltModifier)){
                MainWindow::windowRef->ui->tableEntries->setFocus();
                return true;
            }else if(key == Qt::Key_Up
                     && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                MainWindow::windowRef->on_btnPrevFrag_clicked();
                return true;
            }else if(key == Qt::Key_Down
                     && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                MainWindow::windowRef->on_btnNextFrag_clicked();
                return true;
            }else if(key == Qt::Key_S
                     && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                MainWindow::windowRef->on_btnSaveFrag_clicked();
                return true;
            }
        default:
            return false;
    }
    return false;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::windowRef = this;

    restoreHistory();
    initUI();
    initFilter();

    if(Helper::instance()->mWorkingStatusQuo.contains("projectDirectory")){
        Helper::instance()->mProjectDirectory = Helper::instance()->mWorkingStatusQuo["projectDirectory"];
        this->setWindowTitle("WikiMate @ " + Helper::instance()->mProjectDirectory);
        Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
        updateFileList(files);
    }

    connect(DictEngine::instance(), &DictEngine::receivedEntryResponse, this, &MainWindow::on_receivedEntryResponse);
    connect(&mAsyncBuildFragment, &AsyncBuildFragment::finished, this, &MainWindow::on_fragmentDataReady);
    connect(ui->tbvFiles, &FileTableView::startTransEditing, this, &MainWindow::on_btnStartTask_clicked);
    connect(ui->tbvFiles, &FileTableView::startExportTask, this, &MainWindow::on_btnExportTask_clicked);
    connect(ui->tbvFiles, &FileTableView::refreshTaskList, this, &MainWindow::on_btnRefreshTasks_clicked);
    connect(Helper::instance(), &Helper::refreshTaskList, this, &MainWindow::on_btnRefreshTasks_clicked);
    connect(ui->tbvFiles, &FileTableView::startAddNewTasks, this, &MainWindow::on_btnAddTasks_clicked);
    connect(ui->tbvFiles, &FileTableView::startRemoveTasks, this, &MainWindow::on_btnRemoveTasks_clicked);
}

MainWindow::~MainWindow()
{
    saveHistory();
    delete ui;
}

void MainWindow::initFilter(){
    EntryTableKeyFilter* entryTableKeyFilter = new EntryTableKeyFilter;
    FragmentEditorKeyFilter* fragmentEditorKeyFilter = new FragmentEditorKeyFilter;
    ui->tableEntries->installEventFilter(entryTableKeyFilter);
    ui->txtTrans->installEventFilter(fragmentEditorKeyFilter);
}

void MainWindow::initUI(){
    ui->tabCenter->tabBar()->hide();
    ui->tabLeftSide->tabBar()->hide();
    ui->tabLeftMenu->tabBar()->hide();
    ui->tabTopTools->tabBar()->hide();
    ui->splitter_horizon->setStretchFactor(0, 1);
    ui->splitter_horizon->setStretchFactor(1, 1);
    ui->splitter_vertical->setStretchFactor(0, 3);
    ui->splitter_vertical->setStretchFactor(1, 2);

    QString scrollBarStyle = "QScrollBar{background:transparent; width: 10px;}"
                             "QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
                             "QScrollBar::handle:hover{background:gray;}"
                             "QScrollBar::sub-line{background:transparent;}"
                             "QScrollBar::add-line{background:transparent;}";
    ui->txtOriginal->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->txtOriginal->horizontalScrollBar()->setStyleSheet(scrollBarStyle);

    QListWidgetItem *item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Tasks");
    item->setToolTip("Show files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(35, 35));
    ui->lstMenu->addItem(item);
    ui->lstMenu->setCurrentItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Editor");
    item->setToolTip("Current editing page");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(35, 35));
    ui->lstMenu->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Trans-Mem");
    item->setToolTip("Translation Memory");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(35, 35));
    ui->lstMenu->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("All Tasks");
    item->setToolTip("Display all files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstTaskFilter->addItem(item);
    ui->lstTaskFilter->setCurrentItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Tasks [DOING]");
    item->setToolTip("Display all translating files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstTaskFilter->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Tasks [DONE]");
    item->setToolTip("Display all translated files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstTaskFilter->addItem(item);
}

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

void MainWindow::updateFileList(const QVector<QStringList> &data){
    Helper::instance()->mTaskListBackup = data;
    ui->tbvFiles->updateData(data);
}

void MainWindow::filterFileList(const QString& keyword){
    if(Helper::instance()->mTaskListBackup.size() > 0){
        QVector<QStringList> new_data;
        if(keyword == "DOING"){
            for(auto iter = Helper::instance()->mTaskListBackup.begin();
                iter != Helper::instance()->mTaskListBackup.end();
                iter++){
                if(iter->at(4) == "DOING"){
                    new_data.append(*iter);
                }
            }
        }else if(keyword == "DONE"){
            for(auto iter = Helper::instance()->mTaskListBackup.begin();
                iter != Helper::instance()->mTaskListBackup.end();
                iter++){
                if(iter->at(4) == "DONE"){
                    new_data.append(*iter);
                }
            }
        }else{
            new_data = Helper::instance()->mTaskListBackup;
        }
        ui->tbvFiles->updateData(new_data);
    }
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



void MainWindow::restoreHistory(){
    QString path = "";
    if(QDir::currentPath().endsWith(QDir::separator())){
        path = QDir::currentPath() + "history.meta";
    }else{
        path = QDir::currentPath() + QDir::separator() + "history.meta";
    }
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)){
        QJsonParseError error;
        QByteArray data = file.readAll();
        if (data.isEmpty()){
            return;
        }
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
        if (error.error == QJsonParseError::NoError) {
            if (jsonDocument.isObject()) {
                QVariantMap result = jsonDocument.toVariant().toMap();
                for(auto iter = result.begin(); iter != result.end(); iter++){
                    Helper::instance()->mWorkingStatusQuo[iter.key()] = iter.value().toString();
                }
            }
        } else {
            qDebug() << "failed to parse JSON:" << error.errorString().toUtf8().constData();
            return;
        }
    }
}

void MainWindow::saveHistory(){
    QJsonParseError error;
    QVariantMap variantMap;
    for(auto iter = Helper::instance()->mWorkingStatusQuo.begin(); iter != Helper::instance()->mWorkingStatusQuo.end(); iter++){
        variantMap[iter.key()] = iter.value();
    }
    QJsonObject obj = QJsonObject::fromVariantMap(variantMap);
    QJsonDocument jsonDocument(obj);
    QByteArray data = jsonDocument.toJson();

    QString path = "";
    if(QDir::currentPath().endsWith(QDir::separator())){
        path = QDir::currentPath() + "history.meta";
    }else{
        path = QDir::currentPath() + QDir::separator() + "history.meta";
    }
    QFile file(path);
    if(file.open(QIODevice::WriteOnly)){
        file.write(data);
        file.flush();
        file.close();
    }else{
        qDebug() << "failed to write JSON to file";
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

void AsyncBuildFragment::run(){
    FragmentManager::instance()->buildOrLoadFragments(FragmentManager::instance()->mSourceFilePath);
}

void MainWindow::on_btnStartTask_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        ui->statusLabel->setText("Building fragment map, please wait...");
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mProjectDirectory, fileName);
        qDebug() << "start task:" << path;

        if(FragmentManager::instance()->mSourceFilePath != path){
            FragmentManager::instance()->mSourceFilePath = path;
            qDebug() << "start async build fragments";
            mAsyncBuildFragment.start();

            // FragmentManager::instance()->buildOrLoadFragments(path);
            // setCurrentFragment(0);
        }
    }
}

void MainWindow::on_fragmentDataReady(){
    qDebug() << "fragments data are ready";
    ui->lstMenu->setCurrentRow(1);
    ui->statusLabel->setText("Working on <strong>" +
                             FragmentManager::instance()->mSourceFilePath.mid(
                                 FragmentManager::instance()->mSourceFilePath.lastIndexOf("/")+1) + "</strong>");
    setCurrentFragment(0);
}

void MainWindow::setCurrentFragment(int index){
    qDebug() << "shift to editor page";
    if(FragmentManager::instance()->mFragmentList.isEmpty()){
        return;
    }
    if(index >= FragmentManager::instance()->mFragmentList.length()
            || index < 0){
        return;
    }
    FragmentManager::instance()->mCurrentIndex = index;

    // 设置源文件的HTML格式
    // ui->txtOriginal->setHtml(Helper::instance()->formatContent(FragmentManager::instance()->mFragmentList, index));
    ui->txtOriginal->setHtml(FragmentManager::instance()->getFormatContent());

    // 设置词条数据
    // showEntriesTableAsync(FragmentManager::instance()->currentFragmentWords());
    showEntriesTable(FragmentManager::instance()->currentFragmentWords());

    // 加载已有的翻译
    ui->txtTrans->clear();
    QString trans = FragmentManager::instance()->mFragmentTransList[FragmentManager::instance()->mCurrentIndex];
    if(trans != ""){
        ui->txtTrans->setText(trans);
    }

    ui->tableEntries->itemAt(0, 0)->setSelected(true);
    ui->tableEntries->setFocus(Qt::MouseFocusReason);
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
    /*
    QTextCursor cursor = ui->txtOriginal->textCursor();
    int blockNum = cursor.blockNumber();
    int blockPos = cursor.positionInBlock();
    int word_begin = 0;
    int word_len = 0;
    if(blockNum == 0 && blockPos == 0){
        return;
    }
    */
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
    showEntriesTable(FragmentManager::instance()->currentFragmentWords());
}

void MainWindow::showEntriesTable(const QStringList &header){
    qDebug() << "show entries table";
    QMap<QString, QStringList> entries;
    for(QString word : header){
        entries.insert(word, DictEngine::instance()->fetchEntry(word, "zh", "en"));
    }

    int maxLen = 0;
    for(QString key : entries.keys()){
        if(entries[key].length() > maxLen){
            maxLen = entries[key].length();
        }
    }

    ui->tableEntries->clear();
    ui->tableEntries->setColumnCount(header.size());
    ui->tableEntries->setRowCount(maxLen);
    ui->tableEntries->setHorizontalHeaderLabels(header);

    for(int i = 0; i < header.size(); i++){
        QStringList trans = entries[header.at(i)];
        for(int j = 0; j < trans.size(); j++){
            ui->tableEntries->setItem(j, i, new QTableWidgetItem(trans[j]));
        }
    }
}

void MainWindow::showEntriesTableAsync(const QStringList &header){
    qDebug() << "async-show entries table";
    ui->tableEntries->clear();
    ui->tableEntries->setRowCount(1);
    ui->tableEntries->setColumnCount(header.size());
    ui->tableEntries->setHorizontalHeaderLabels(header);

    /*for(QString word : header){
        DictEngine::instance()->fetchEntryAsync(word, "zh", "en");
    }*/
     DictEngine::instance()->fetchEntryAsync("这个是", "zh", "en");
     //DictEngine::instance()->fetchEntryAsync("朱万利", "zh", "en");
     //DictEngine::instance()->fetchEntryAsync("爱新觉罗", "zh", "en");
     //DictEngine::instance()->fetchEntryAsync("释迦摩尼", "zh", "en");
}

void MainWindow::on_receivedEntryResponse(QString word, QStringList trans){
    if(word.isEmpty()){
        return;
    }
    for(int i = 0; i < ui->tableEntries->columnCount(); i++){
        if(word == ui->tableEntries->horizontalHeaderItem(i)->text()){
            while(trans.length() > ui->tableEntries->rowCount()){
                ui->tableEntries->insertRow(ui->tableEntries->rowCount());
            }
            for(int j = 0; j < trans.length(); j++){
                ui->tableEntries->setItem(j, i, new QTableWidgetItem(trans[j]));
            }
        }
    }
}

void MainWindow::on_txtOriginal_customContextMenuRequested(const QPoint &pos)
{

}

void MainWindow::on_tableEntries_itemChanged(QTableWidgetItem *item)
{

}
