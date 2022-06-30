#ifndef FRMTEMPERATUREPLOT_H
#define FRMTEMPERATUREPLOT_H

#include "qdatetime.h"
#include <QWidget>

namespace Ui {
class FrmTemperatureplot;
}

class FrmTemperatureplot : public QWidget
{
    Q_OBJECT

public:
    static FrmTemperatureplot & GetInstance()
    {
        static FrmTemperatureplot m_instance; // 静态局部对象的引用
        return m_instance;
    }
    explicit FrmTemperatureplot(QWidget *parent = nullptr);
    ~FrmTemperatureplot();

    void drawDataPoint(quint8 sensorNum, QVector<float> &mTemp);
    void clearData();

private:
    Ui::FrmTemperatureplot *ui;
    QTime timeStart;

private slots:
    void initForm();
};

#endif // FRMTEMPERATUREPLOT_H
