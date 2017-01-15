#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUI();
    auto a = QList<QMap<QString, QString> >();
    on_update_fileList(a);
}

MainWindow::~MainWindow()
{
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

void MainWindow::on_update_fileList(const QList<QMap<QString, QString> > &data){

}
