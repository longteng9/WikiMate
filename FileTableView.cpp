#include "FileTableView.h"
#include "FileItemDelegate.h"
#include "FileTableModel.h"
#include <QHeaderView>
#include <QMenu>
#include <QDebug>
#include <QDir>

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
    headers << "" << "Filename" << "Words" << "Size" << "State" << "Completion Rate" << "Tag" << "Start Time";
    mModel->setHorizontalHeader(headers);

    mItemDelegate = new FileItemDelegate(this);
    this->setItemDelegate(mItemDelegate);
    emit mModel->layoutChanged();
    this->setColumnWidth(0, 35 + 4);
    this->setColumnWidth(1, 190);
    this->setColumnWidth(2, 70);
    this->setColumnWidth(3, 70);
    this->setColumnWidth(4, 75);
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
    mContextMenu->setStyleSheet("QMenu { font: 11pt \"Verdana\"; padding: 1 2 1 2;}");

    QAction *act_trans = mContextMenu->addAction("Open and Trans");
    QAction *act_finalize = mContextMenu->addAction("Close and Finalize");
    QAction *act_addFiles = mContextMenu->addAction("Add Files");
    QAction *act_delFile = mContextMenu->addAction("Delete File");
    QAction *act_openFolder = mContextMenu->addAction("Open Folder");
    QAction *act_refreshList = mContextMenu->addAction("Refresh List");
    connect(act_trans, &QAction::triggered, this, &FileTableView::act_openAndTrans);
    connect(act_finalize, &QAction::triggered, this, &FileTableView::act_closeAndFinalize);
    connect(act_addFiles, &QAction::triggered, this, &FileTableView::act_addFiles);
    connect(act_delFile, &QAction::triggered, this, &FileTableView::act_deleteFile);
    connect(act_openFolder, &QAction::triggered, this, &FileTableView::act_openFolder);
    connect(act_refreshList, &QAction::triggered, this, &FileTableView::act_refreshList);

    mContextMenu->exec(QCursor::pos());
}

void FileTableView::onDoubleClicked(const QModelIndex &index){
    QString path = QDir::currentPath();
    QString fileName = mModel->index(index.row(), 1).data().toString();
    if(path.endsWith(QDir::separator())){
        path.append(fileName);
    }else{
        path.append(QDir::separator());
        path.append(fileName);
    }
    emit startTransEditing(path);
}

void FileTableView::act_openAndTrans(){
    int row = this->currentIndex().row();

}

void FileTableView::act_closeAndFinalize(){
    int row = this->currentIndex().row();

}

void FileTableView::act_addFiles(){
    int row = this->currentIndex().row();

}

void FileTableView::act_deleteFile(){
    int row = this->currentIndex().row();

}

void FileTableView::act_openFolder(){
    int row = this->currentIndex().row();

}

void FileTableView::act_refreshList(){

}


