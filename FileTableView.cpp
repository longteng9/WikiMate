#include "FileTableView.h"
#include "FileItemDelegate.h"
#include "FileTableModel.h"
#include <QHeaderView>
#include <QMenu>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include "MainWindow.h"
#include "Helper.h"
#include "MessageForm.h"

FileTableView::FileTableView(QWidget *parent) :
    QTableView(parent),
    mContextMenu(NULL)
{
    initUI();
    initData();
    connect(this, &FileTableView::customContextMenuRequested, this, &FileTableView::onCreateContextMenu);
    connect(this, &QTableView::doubleClicked, this, &FileTableView::onDoubleClicked);
}

FileTableView::~FileTableView()
{
    delete mModel;
}


void FileTableView::initUI(){
    this->verticalHeader()->setVisible(false);
    this->setShowGrid(false);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode ( QAbstractItemView::SingleSelection);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->verticalHeader()->setDefaultSectionSize(35);
    this->setFocusPolicy(Qt::NoFocus);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

void FileTableView::initData()
{
    mModel = new FileTableModel();
    this->setModel(mModel);
    QStringList headers;
    headers << "" << "Filename" << "Words" << "Size" << "State" << "Completion Rate" << "Encoding" << "Start Time";
    mModel->setHorizontalHeader(headers);

    mItemDelegate = new FileItemDelegate(this);
    this->setItemDelegate(mItemDelegate);
    emit mModel->layoutChanged();
    this->setColumnWidth(0, 35 + 4);
    this->setColumnWidth(1, 240);
    this->setColumnWidth(2, 70);
    this->setColumnWidth(3, 90);
    this->setColumnWidth(4, 70);
    this->setColumnWidth(5, 180);
    this->setColumnWidth(6, 70);
}

void FileTableView::updateData(const QVector<QStringList> &data){
    mModel->setData(data);
    emit mModel->layoutChanged();
}

void FileTableView::onCreateContextMenu(const QPoint &point){
    if (mContextMenu != NULL){
        delete mContextMenu;
        mContextMenu = NULL;
    }
    mContextMenu = new QMenu(this);
    mContextMenu->setStyleSheet("QMenu { font: 10pt \"Verdana\"; padding: 1 2 1 2;}");

    QAction *act_trans = mContextMenu->addAction("Start Task");
    QAction *act_finalize = mContextMenu->addAction("Export Task");
    QAction *act_addFiles = mContextMenu->addAction("Add New Task");
    QAction *act_delFile = mContextMenu->addAction("Remove Task");
    QAction *act_openFolder = mContextMenu->addAction("Open Folder");
    QAction *act_refreshList = mContextMenu->addAction("Refresh List");
    connect(act_trans, &QAction::triggered, this, &FileTableView::act_startTask);
    connect(act_finalize, &QAction::triggered, this, &FileTableView::act_exportTask);
    connect(act_addFiles, &QAction::triggered, this, &FileTableView::act_addTasks);
    connect(act_delFile, &QAction::triggered, this, &FileTableView::act_removeTasks);
    connect(act_openFolder, &QAction::triggered, this, &FileTableView::act_openFolder);
    connect(act_refreshList, &QAction::triggered, this, &FileTableView::act_refreshList);

    mContextMenu->exec(QCursor::pos());
}

void FileTableView::onDoubleClicked(const QModelIndex &index){
    QString format = mModel->index(index.row(), 6).data().toString();
    if(format != "UTF-8"){
        /*int result = QMessageBox::warning(NULL,
                             "Warning",
                             "This file isn't encoded in UTF-8,\nthere might be a problem to decode this file as UTF-8,\ndo you wanna try?",
                             QMessageBox::Yes,
                             QMessageBox::No);
        if(result == QMessageBox::No){
            return;
        }*/
        MessageForm::createAndShowAs(MessageForm::Role::QueryDialogForm,
                                     "Notice", "Current file:\n"
                                     + mModel->index(index.row(), 1).data().toString()
                                     + "\n\ndoes not encoded in UTF-8, there might not be able to decode this file correctly.\nDo you want to try, anyway?",
                                     [this](bool positive){
            if(positive){
                emit this->startTransEditing();
            }
        });
    }else{
        emit startTransEditing();
    }
}

void FileTableView::act_startTask(){
    int row = this->currentIndex().row();
    if(row >= 0){
        emit startTransEditing();
    }

}

void FileTableView::act_exportTask(){
    int row = this->currentIndex().row();
    if (row > 0){
        emit startExportTask();
    }
}

void FileTableView::act_addTasks(){
    emit startAddNewTasks();
}

void FileTableView::act_removeTasks(){
    int row = this->currentIndex().row();
    emit startRemoveTasks();
}

void FileTableView::act_openFolder(){

}

void FileTableView::act_refreshList(){
    emit refreshTaskList();
}


