#include "FileTableView.h"
#include "FileItemDelegate.h"
#include "FileTableModel.h"
#include <QHeaderView>
#include <QMenu>
#include <QDebug>

FileTableView::FileTableView(QWidget *parent) :
    QTableView(parent),
    mContextMenu(NULL)
{
    initUI();
    initData();
    connect(this, &FileTableView::customContextMenuRequested, this, &FileTableView::onCreateContextMenu);
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
    headers << "" << "Filename" << "Words" << "Size" << "State" << "Progress" << "Tag" << "Start Time";
    mModel->setHorizontalHeader(headers);


    QVector<QStringList> data;
    data.append(QStringList() << ":/static/question.png" << "filename1" << "123" << "872KB" << "Translating" << "50" << "" << "2017-02-01");
    data.append(QStringList() << ":/static/question.png" << "filename2" << "123" << "872KB" << "Translating" << "50"  << "" << "2017-02-01");
    data.append(QStringList() << ":/static/questions.png" << "filename3" << "123" << "872KB" << "Translating" << "50"  << "" << "2017-02-01");
    data.append(QStringList() << ":/static/question.png" << "filename4" << "123" << "872KB" << "Translating" << "50" << "" << "2017-02-01");
    data.append(QStringList() << ":/static/questions.png" << "filename5" << "123" << "872KB" << "Translating" << "50"  << "" << "2017-02-01");
    mModel->setData(data);

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
    connect(act_trans, &QAction::triggered, this, &FileTableView::act_openAndTrans);
    connect(act_finalize, &QAction::triggered, this, &FileTableView::act_closeAndFinalize);
    connect(act_addFiles, &QAction::triggered, this, &FileTableView::act_addFiles);
    connect(act_delFile, &QAction::triggered, this, &FileTableView::act_deleteFile);
    connect(act_openFolder, &QAction::triggered, this, &FileTableView::act_openFolder);

    mContextMenu->exec(QCursor::pos());
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


