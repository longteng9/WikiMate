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

void AsyncBuildFragment::start(){
    qDebug() << "building fragment thread:" << QThread::currentThreadId();
    FragmentManager::instance()->buildOrLoadFragments(FragmentManager::instance()->mSourceFilePath);
    emit finished();
    QThread::currentThread()->quit();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mLauncher = new Launcher;
    mMessageForm = new MessageForm;

    qDebug() << "ui thread:" << QThread::currentThreadId();
    restoreHistory();
    initUI();

    if(Helper::instance()->mWorkingStatusQuo.contains("projectDirectory")){
        Helper::instance()->mProjectDirectory = Helper::instance()->mWorkingStatusQuo["projectDirectory"];
        this->setWindowTitle("WikiMate @ " + Helper::instance()->mProjectDirectory);
        ui->statusLabel->setText("Project folder: " + Helper::instance()->mProjectDirectory);
        Helper::instance()->refreshWorkingDir(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
        QVector<QStringList> files = Helper::instance()->getWorkingFiles(Helper::instance()->mWorkingStatusQuo["projectDirectory"]);
        updateFileList(files);
    }

    ui->tableEntries->installEventFilter(this);
    ui->txtTrans->installEventFilter(this);
    ui->txtOriginal->installEventFilter(this);

    connect(DictEngine::instance(), &DictEngine::receivedEntryResponse, this, &MainWindow::on_receivedEntryResponse);
    connect(DictEngine::instance(), &DictEngine::queryDumpEntryFinished, this, &MainWindow::on_receivedEntryResponse);
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
    if(mLauncher!=NULL){
        delete mLauncher;
    }
    if(mMessageForm != NULL){
        delete mMessageForm;
    }
}


bool MainWindow::eventFilter(QObject *watched, QEvent *event){
    int key;
    if(watched == ui->tableEntries){
        switch(event->type())  {
            case QEvent::KeyPress:
                key = (static_cast<QKeyEvent *>(event))->key();
                if (key == Qt::Key_I)  {
                    if(ui->tableEntries->editTriggers() != QAbstractItemView::NoEditTriggers){
                        ui->tableEntries->editItem(ui->tableEntries->item(ui->tableEntries->currentRow(), ui->tableEntries->currentColumn()));
                    }
                    return true;
                }else if(key == Qt::Key_Enter
                         || key == Qt::Key_Return){
                    if(!ui->tableEntries->item(ui->tableEntries->currentRow(), ui->tableEntries->currentColumn())){
                        return false;
                    }
                    QString entry = ui->tableEntries->item(ui->tableEntries->currentRow(), ui->tableEntries->currentColumn())->text();
                    ui->txtTrans->setText(ui->txtTrans->toPlainText() + entry + " ");
                    return true;
                }else if(key == Qt::Key_Down
                         && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::AltModifier)){
                    ui->txtTrans->setFocus();
                    QTextCursor cursor = ui->txtTrans->textCursor();
                    cursor.movePosition(QTextCursor::MoveOperation::EndOfBlock);
                    ui->txtTrans->setTextCursor(cursor);
                    return true;
                }else if(key == Qt::Key_S
                         && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){

                    return true;
                }
            default:
                return false;
        }
        return false;
    }else if(watched == ui->txtTrans){
        switch(event->type())  {
            case QEvent::KeyPress:
                key = (static_cast<QKeyEvent *>(event))->key();
                if(key == Qt::Key_Up
                         && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::AltModifier)){
                    ui->tableEntries->setFocus();
                    return true;
                }else if(key == Qt::Key_Up
                         && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                    on_btnPrevFrag_clicked();
                    return true;
                }else if(key == Qt::Key_Down
                         && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                    on_btnNextFrag_clicked();
                    return true;
                }else if(key == Qt::Key_S
                         && ((static_cast<QKeyEvent *>(event))->modifiers() & Qt::ControlModifier)){
                    on_btnSaveFrag_clicked();
                    return true;
                }
            default:
                return false;
        }
        return false;
    }else if(watched == ui->txtOriginal){
        switch(event->type()){
        case QEvent::KeyPress:
            key = (static_cast<QKeyEvent *>(event))->key();
            if(key == Qt::Key_A){
                if(mOriginSelection != ""){
                    qDebug() << "show form";
                    mMessageForm->showAs(MessageForm::Role::AddTransMem, mOriginSelection);
                }
                return true;
            }
        default:
            return false;
        }
        return false;
    }
    return false;
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
    item->setIcon(QIcon(":/static/task-icon.ico"));
    item->setText("Tasks");
    item->setToolTip("Show files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(35, 35));
    ui->lstMenu->addItem(item);
    ui->lstMenu->setCurrentItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/editor-icon.png"));
    item->setText("Editor");
    item->setToolTip("Current editing page");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(35, 35));
    ui->lstMenu->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/dictionary-icon.png"));
    item->setText("Trans-Mem");
    item->setToolTip("Translation Memory");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(35, 35));
    ui->lstMenu->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/folder-icon2.png"));
    item->setText("All Tasks");
    item->setToolTip("Display all files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstTaskFilter->addItem(item);
    ui->lstTaskFilter->setCurrentItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/folder-icon2.png"));
    item->setText("Tasks [DOING]");
    item->setToolTip("Display all translating files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstTaskFilter->addItem(item);

    item = new QListWidgetItem;
    item->setIcon(QIcon(":/static/folder-icon2.png"));
    item->setText("Tasks [DONE]");
    item->setToolTip("Display all translated files in workspace");
    item->setTextAlignment(Qt::AlignVCenter);
    item->setBackground(Qt::white);
    item->setSizeHint(QSize(25, 25));
    ui->lstTaskFilter->addItem(item);

    this->ui->btnPrevFrag->setToolTip("Wait until querying process finished");
    this->ui->btnNextFrag->setToolTip("Wait until querying process finished");
    this->ui->btnToggleOD->setToolTip("Only using Wiktionary dump while OD disabled");

    this->ui->btnExportTask->setEnabled(false);
    this->ui->btnExportTask->setToolTip("Please do this in editor mode");
    this->ui->tableEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
}





