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
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingHistory["projectDirectory"]);
        updateFileList(files);
    }

    connect(ui->tbvFiles, &FileTableView::startTransEditing, this, &MainWindow::on_startTransEditing);
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
    item->setText("Files");
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
    item->setText("All Files");
    item->setToolTip("Display all files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstFileOptions->addItem(item);
    ui->lstFileOptions->setCurrentItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Translated Files");
    item->setToolTip("Display all files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstFileOptions->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/question.png"));
    item->setText("Translating Files");
    item->setToolTip("Display all files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstFileOptions->addItem(item);
}

void MainWindow::on_lstMenu_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous){
        previous->setBackground(Qt::white);
    }
    current->setBackgroundColor(QColor("#0F7DBE"));
}

void MainWindow::on_lstFileOptions_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous){
        previous->setBackground(Qt::white);
    }
    current->setBackgroundColor(QColor("#0F7DBE"));
}

void MainWindow::updateFileList(const QVector<QStringList> &data){
    ui->tbvFiles->updateData(data);
}

void MainWindow::on_btnWorkingDir_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Choose Working Directory", "./");
    if(!dirPath.isEmpty()){
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(dirPath);
        updateFileList(files);
        Helper::instance()->mWorkingHistory["workingDirectory"] = dirPath;
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

void MainWindow::on_startTransEditing(const QString &path){
    qDebug() << "start:"<<path;
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
