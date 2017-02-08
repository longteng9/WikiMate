#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowIcon(QIcon(":/static/app_icon_256.ico"));
    w.show();

    return a.exec();
}

/*
修改编辑界面editor tab下的UI结构
1. 左边任然是原文，但是是以段为单位进行高亮，用键盘进行上下高亮段的切换
2. 右边的结构分上下两部分，上面部分是一个2行N列的类似于表的结构，用户用键盘的方向键进行词条意思的选择，用户选择完一段之后，确认该段翻译完时左边的高亮段下移
*/
