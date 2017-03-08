#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QVariantMap>
#include "DictEngine.h"
#include "Helper.h"
#include "FragmentManager.h"
#include <QStandardPaths>
#include <QCoreApplication>

void MainWindow::restoreHistory(){
    QString path = "";
    if(QDir::currentPath().endsWith(QDir::separator())){
        path = QCoreApplication::applicationDirPath() + "/history.meta";
    }else{
        path = QCoreApplication::applicationDirPath() + "/history.meta";
    }
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)){
        QJsonParseError error;
        QByteArray data = file.readAll();
        if (!data.isEmpty()){
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);
            if (error.error == QJsonParseError::NoError) {
                if (jsonDocument.isObject()) {
                    QVariantMap result = jsonDocument.toVariant().toMap();
                    for(auto iter = result.begin(); iter != result.end(); iter++){
                        Helper::instance()->mWorkingStatusQuo[iter.key()] = iter.value().toString();
                    }
                }
            } else {
                qDebug() << "failed to parse JSON:" << error.errorString().toUtf8().constData();
                return;
            }
        }//if (!data.isEmpty())
    }//if(file.open(QIODevice::ReadOnly))
    if(!Helper::instance()->mWorkingStatusQuo.contains("projectDirectory")){
        Helper::instance()->mWorkingStatusQuo["projectDirectory"] = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);;
    }
}

void MainWindow::saveHistory(){
    QJsonParseError error;
    QVariantMap variantMap;
    for(auto iter = Helper::instance()->mWorkingStatusQuo.begin(); iter != Helper::instance()->mWorkingStatusQuo.end(); iter++){
        variantMap[iter.key()] = iter.value();
    }
    QJsonObject obj = QJsonObject::fromVariantMap(variantMap);
    QJsonDocument jsonDocument(obj);
    QByteArray data = jsonDocument.toJson();

    QString path = "";
    if(QDir::currentPath().endsWith(QDir::separator())){
        path = QCoreApplication::applicationDirPath() + "/history.meta";
    }else{
        path = QCoreApplication::applicationDirPath() + "/history.meta";
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








