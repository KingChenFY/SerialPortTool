#include "frmcomtool.h"
#include "quihelper.h"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置编码以及加载中文翻译文件
    QUIHelper::initAll();
    //读取配置文件
    AppConfig::ConfigFile = QString("%1/%2.ini").arg(QUIHelper::appPath()).arg(QUIHelper::appName());
    AppConfig::readConfig();

    frmComTool w;
    w.setWindowTitle("serial port tool By.FY Ver.0");
    w.show();
    return a.exec();
}
