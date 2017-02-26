#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QEvent>
#include <QMouseEvent>
#include <QTextCursor>
#include <QString>
#include <QListWidgetItem>
#include <QIcon>
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
}



void MainWindow::showEntriesTableAsync(const QStringList &header){
    qDebug() << "show entries table asynchronously";
    ui->tableEntries->clear();
    ui->tableEntries->setRowCount(1);
    ui->tableEntries->setColumnCount(header.size());
    ui->tableEntries->setHorizontalHeaderLabels(header);

    DictEngine::instance()->fetchEntryPatchAsync(header, "zh", "en");
}
