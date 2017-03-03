#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QEvent>
#include <QMouseEvent>
#include <QTextCursor>
#include <QString>
#include <QListWidgetItem>
#include <QIcon>
#include <QString>
#include <QScrollBar>
#include "DictEngine.h"
#include "FragmentManager.h"

void MainWindow::setCurrentFragment(int index){
    if(FragmentManager::instance()->mFragmentList.isEmpty()){
        return;
    }
    if(index >= FragmentManager::instance()->mFragmentList.length()
            || index < 0){
        return;
    }
    FragmentManager::instance()->mCurrentIndex = index;

    // 设置源文件的HTML格式
    ui->txtOriginal->setHtml(FragmentManager::instance()->getFormatContent());

    // 设置词条数据
    showEntriesTableAsync(FragmentManager::instance()->currentFragmentWords());

    // 加载已有的翻译
    ui->txtTrans->clear();
    QString trans = FragmentManager::instance()->mFragmentTransList[FragmentManager::instance()->mCurrentIndex];
    if(trans != ""){
        ui->txtTrans->setText(trans);
    }

    if(ui->tableEntries->itemAt(0, 0) != NULL){
        ui->tableEntries->itemAt(0, 0)->setSelected(true);
        ui->tableEntries->setFocus(Qt::MouseFocusReason);
    }

    if(FragmentManager::instance()->jiebaValid()){
        emit closeLoadingForm();
    }
}

void MainWindow::showEntriesTableAsync(const QStringList &header){
    if(!ui->btnNextFrag->isEnabled()){
        qDebug() << "wait for querying finished";
        ui->statusLabel->setText("Please wait for querying process finished");
        return;
    }
    ui->tableEntries->clear();
    ui->tableEntries->setRowCount(1);
    ui->tableEntries->setColumnCount(header.size());
    ui->tableEntries->setHorizontalHeaderLabels(header);

    qDebug() << "load entry table asynchronously";
    DictEngine::instance()->stopQueryAndFetch();
    mReceivedEntryCount = 0;
    this->ui->btnNextFrag->setEnabled(false);
    this->ui->btnPrevFrag->setEnabled(false);

    ui->statusLabel->setText("Loading entry table asynchronously");
    DictEngine::instance()->queryDumpEntryPatchAsync(header, "zh", "en");
#ifndef BLOCK_NET_ENTRY_QUERY
    if(mEnableOnlineDict){
        DictEngine::instance()->fetchEntryPatchAsync(header, "zh", "en");
    }
#endif
}

void MainWindow::showTransMemTable(){
    ui->tabCenter->setCurrentIndex(2);
    ui->tabLeftSide->setCurrentIndex(2);
    ui->tabTopTools->setCurrentIndex(2);
    ui->transMemTable->clear();
    QMap<QString, QStringList> result = DictEngine::instance()->getAllTransMem();

    int maxCol = 1;
    for(QString key : result.keys()){
        if(result[key].length() > maxCol){
            maxCol = result[key].length();
        }
    }
    maxCol += 1;

    ui->transMemTable->setColumnCount(maxCol);
    ui->transMemTable->setRowCount(result.keys().length());
    int row = 0;
    for(QString key : result.keys()){
        ui->transMemTable->setItem(row, 0, new QTableWidgetItem(key));
        QStringList transList = result[key];
        for(int j = 0; j < transList.size(); j++){
            ui->transMemTable->setItem(row, 1 + j, new QTableWidgetItem(transList[j]));
        }
        row += 1;
    }
}


void MainWindow::autoScrollOriginBrowser(int curFragId){
    double rate = 0;
    int curBlockLen = 0;
    int allBlocksLen = 0;
    for(int i = 0; i < FragmentManager::instance()->mFragmentList.length(); i++){
        allBlocksLen += FragmentManager::instance()->mFragmentList[i].length();
        allBlocksLen += FragmentManager::instance()->mFragmentTransList.length();
        if(i <= curFragId){
            curBlockLen += FragmentManager::instance()->mFragmentList[i].length();
            curBlockLen += FragmentManager::instance()->mFragmentTransList.length();
        }
    }
    rate = (double)curBlockLen / allBlocksLen;
    qDebug() <<"origin browser current block length:"<<curBlockLen;
    qDebug() <<"origin browser all blocks length:" <<allBlocksLen;
    qDebug() << "rate:"<<rate;
    if(rate == 0){
        return;
    }
    QScrollBar *scrollbar = ui->txtOriginal->verticalScrollBar();
    scrollbar->setSliderPosition((scrollbar->maximum() - scrollbar->minimum()) * rate);
}
