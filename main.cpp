#include "frmcomtool.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    frmComTool w;
    w.show();
    return a.exec();
}
