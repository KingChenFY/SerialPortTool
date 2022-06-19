#ifndef FRMCOMTOOL_H
#define FRMCOMTOOL_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class frmComTool; }
QT_END_NAMESPACE

class frmComTool : public QWidget
{
    Q_OBJECT

public:
    frmComTool(QWidget *parent = nullptr);
    ~frmComTool();

private:
    Ui::frmComTool *ui;
};
#endif // FRMCOMTOOL_H
