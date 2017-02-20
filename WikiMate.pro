#-------------------------------------------------
#
# Project created by QtCreator 2017-01-10T16:53:26
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WikiMate
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# INCLUDEPATH += include/
# LIBS += D:/WikiMate/libs/python36.lib

SOURCES += main.cpp\
        MainWindow.cpp \
    FileTableModel.cpp \
    FileTableView.cpp \
    FileItemDelegate.cpp \
    Helper.cpp \
    FragmentManager.cpp \
    SocketComm.cpp

HEADERS  += MainWindow.h \
    FileTableModel.h \
    FileTableView.h \
    FileItemDelegate.h \
    Helper.h \
    FragmentManager.h \
    SocketComm.h

FORMS    += MainWindow.ui

RESOURCES += \
    resource.qrc
