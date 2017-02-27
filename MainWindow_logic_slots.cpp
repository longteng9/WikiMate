#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>

 void MainWindow::on_buildFragmentFinished(){
    setCurrentFragment(0);
    this->mMessageForm->hide();
}

void MainWindow::on_receivedEntryResponse(QString word, QStringList trans){
    if(word.isEmpty()){
        return;
    }
    for(int i = 0; i < ui->tableEntries->columnCount(); i++){
        if(word == ui->tableEntries->horizontalHeaderItem(i)->text()){
            // 如果当前行数不够，则增加
            while(trans.length() > ui->tableEntries->rowCount()){
                ui->tableEntries->insertRow(ui->tableEntries->rowCount());
            }
            for(int j = 0; j < ui->tableEntries->rowCount(); j++){
                if(j < trans.length()){
                    ui->tableEntries->setItem(j, i, new QTableWidgetItem(trans[j]));
                }else{ // 清除旧数据
                    ui->tableEntries->setItem(j, i, new QTableWidgetItem(""));
                }
            }
        }
    }
}
