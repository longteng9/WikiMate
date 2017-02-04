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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    restoreHistory();
    initUI();
    if(Helper::instance()->mWorkingHistory.contains("projectDirectory")){
        Helper::instance()->mCurrenttDirectory = Helper::instance()->mWorkingHistory["projectDirectory"];
        this->setWindowTitle("WikiMate => " + Helper::instance()->mCurrenttDirectory);
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

void MainWindow::on_lstFileOptions_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
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

void MainWindow::on_lstMenu_clicked(const QModelIndex &index)
{
    if(index.data().toString() == "Files"){
        ui->tabCenter->setCurrentIndex(0);
    } else if(index.data().toString() == "Editor"){
        ui->tabCenter->setCurrentIndex(1);
        QModelIndex fileIndex = ui->tbvFiles->currentIndex();
        if(fileIndex.row() >= 0){
            QString path = Helper::instance()->pathJoin(Helper::instance()->mCurrenttDirectory,
                                                        fileIndex.model()->index(fileIndex.row(), 1).data().toString());
            //todo editor ui
        }
    } else if(index.data().toString() == "Trans-Mem"){
        ui->tabCenter->setCurrentIndex(2);
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
        ui->tabCenter->setCurrentIndex(1);
        ui->tabLeftSide->setCurrentIndex(1);
        ui->tabTopTools->setCurrentIndex(1);

        if(Helper::instance()->mCurrentTaskPath != path){
            QStringList sentences = Helper::instance()->readForSentences(path);
            QString content = "";
            for(QString sen: sentences){
               content += sen + "\n\n";
            }
            ui->txtOriginal->setText(content);
            Helper::instance()->mCurrentTaskPath = path;
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
    QTextCursor cursor = ui->txtOriginal->textCursor();
    int blockNum = cursor.blockNumber();
    int blockPos = cursor.positionInBlock();

}

void MainWindow::on_txtOriginal_selectionChanged()
{

}
