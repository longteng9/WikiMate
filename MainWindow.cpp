#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include <QStandardItemModel>
#include "FileTableModel.h"
#include "Helper.h"
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTextBlock>
#include <QModelIndex>
#include "FragmentManager.h"

QWidget* MainWindow::widgetRef = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::widgetRef = this;
    restoreHistory();
    initUI();
    if(Helper::instance()->mWorkingHistory.contains("projectDirectory")){
        Helper::instance()->mCurrenttDirectory = Helper::instance()->mWorkingHistory["projectDirectory"];
        this->setWindowTitle("WikiMate @ " + Helper::instance()->mCurrenttDirectory);
        Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingHistory["projectDirectory"]);
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingHistory["projectDirectory"]);
        updateFileList(files);
    }

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

void MainWindow::initUI(){
    ui->tabCenter->tabBar()->hide();
    ui->tabLeftSide->tabBar()->hide();
    ui->tabLeftMenu->tabBar()->hide();
    ui->tabTopTools->tabBar()->hide();
    ui->splitter_horizon->setStretchFactor(0, 1);
    ui->splitter_horizon->setStretchFactor(1, 1);
    ui->splitter_vertical->setStretchFactor(0, 3);
    ui->splitter_vertical->setStretchFactor(1, 2);

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
        Helper::instance()->mWorkingHistory["projectDirectory"] = dirPath;
        Helper::instance()->refreshWorkingDir(dirPath);
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
                    Helper::instance()->mWorkingHistory[iter.key()] = iter.value().toString();
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
    for(auto iter = Helper::instance()->mWorkingHistory.begin(); iter != Helper::instance()->mWorkingHistory.end(); iter++){
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
    Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingHistory["projectDirectory"]);
    QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingHistory["projectDirectory"]);
    updateFileList(files);
}

void MainWindow::on_btnStartTask_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mCurrenttDirectory, fileName);
        qDebug() << path;
        ui->lstMenu->setCurrentRow(1);

        if(Helper::instance()->mCurrentTaskPath != path){
            FragmentManager::instance()->buildFragments(path);

            Helper::instance()->mCurrentTaskPath = path;
            setCurrentFragment(0);
        }
    }
}

void MainWindow::setCurrentFragment(int index){
    if(FragmentManager::instance()->mFragmentList.isEmpty()){
        return;
    }
    ui->txtOriginal->setHtml(Helper::instance()->formatContent(FragmentManager::instance()->mFragmentList, index));

    QStringList header;
    for(QString word : FragmentManager::instance()->currentBlockFragments()){
        header << word;
    }

    QMap<QString, QStringList> transMap = Helper::instance()->searchTrans(header);
    int maxLen = 0;
    for(QString key : transMap.keys()){
        if(transMap[key].length() > maxLen){
            maxLen = transMap[key].length();
        }
    }

    ui->tableWords->clear();
    ui->tableWords->setColumnCount(header.size());
    ui->tableWords->setRowCount(maxLen);

    ui->tableWords->setHorizontalHeaderLabels(header);
    for(int i = 0; i < header.size(); i++){
        QStringList entries = transMap[header.at(i)];
        for(int j = 0; j < entries.size(); j++){
            ui->tableWords->setItem(j, i, new QTableWidgetItem(entries[j]));
        }
    }
}

void MainWindow::on_btnExportTask_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mCurrenttDirectory, fileName);
        qDebug() << path;
    }
}

void MainWindow::on_btnAddTasks_clicked()
{
    Helper::instance()->addNewTasks(this);
    Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingHistory["projectDirectory"]);
    QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingHistory["projectDirectory"]);
    updateFileList(files);
}

void MainWindow::on_btnRemoveTasks_clicked()
{
    if(ui->tbvFiles->currentIndex().row() >= 0){
        QString fileName = ui->tbvFiles->tableModel()->index(ui->tbvFiles->currentIndex().row(), 1).data().toString();
        QString path = Helper::instance()->pathJoin(Helper::instance()->mCurrenttDirectory, fileName);
        QFile::remove(path);
        Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingHistory["projectDirectory"]);
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingHistory["projectDirectory"]);
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
