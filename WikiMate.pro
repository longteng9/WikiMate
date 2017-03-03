#-------------------------------------------------
#
# Project created by QtCreator 2017-01-10T16:53:26
#
#-------------------------------------------------

QT       += core gui xml network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WikiMate
TEMPLATE = app

# disables all the APIs deprecated before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
# emit warnings if you use any deprecated feature
DEFINES += QT_DEPRECATED_WARNINGS \
    BLOCK_NET_ENTRY_QUERY

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$INCLUDEPATH
# message($$DEPENDPATH)

CONFIG += debug_and_release \
    c++11

DEFINES += ASIO_STANDALONE

CONFIG(debug, debug|release){
    DESTDIR = debug
    UI_DIR = tmp/debug_ui
    MOC_DIR = tmp/debug_moc
    RCC_DIR = tmp/debug_rcc
    OBJECTS_DIR = tmp/debug_obj
}else{
    DESTDIR = release
    UI_DIR = tmp/release_ui
    MOC_DIR = tmp/release_moc
    RCC_DIR = tmp/release_rcc
    OBJECTS_DIR = tmp/release_obj
}

win32:{
    LIBS += -lws2_32
}
unix:{
}

SOURCES += main.cpp\
        MainWindow.cpp \
    FileTableModel.cpp \
    FileTableView.cpp \
    FileItemDelegate.cpp \
    Helper.cpp \
    FragmentManager.cpp \
    DictEngine.cpp \
    Request.cpp \
    MessageForm.cpp \
    Launcher.cpp \
    MainWindow_logic_func.cpp \
    MainWindow_logic_slots.cpp \
    MainWindow_ui_func.cpp \
    MainWindow_ui_slots.cpp

HEADERS  += MainWindow.h \
    FileTableModel.h \
    FileTableView.h \
    FileItemDelegate.h \
    Helper.h \
    FragmentManager.h \
    DictEngine.h \
    Request.h \
    MessageForm.h \
    Launcher.h


FORMS    += MainWindow.ui \
    MessageForm.ui

RESOURCES += \
    resource.qrc


