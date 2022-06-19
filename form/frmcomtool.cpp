#include "frmcomtool.h"
#include "ui_frmcomtool.h"

frmComTool::frmComTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::frmComTool)
{
    ui->setupUi(this);
}

frmComTool::~frmComTool()
{
    delete ui;
}

