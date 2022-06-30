#include "frmtemperatureplot.h"
#include "ui_frmtemperatureplot.h"

FrmTemperatureplot::FrmTemperatureplot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmTemperatureplot)
{
    ui->setupUi(this);
    this->initForm();
}

FrmTemperatureplot::~FrmTemperatureplot()
{
    delete ui;
}

void FrmTemperatureplot::initForm()
{
    timeStart = QTime::currentTime();
    QList<QColor> color;
    color << Qt::red << Qt::green << Qt::blue << Qt::yellow << Qt::black << Qt::gray;

    for(quint8 i = 0; i < 6; i++)
    {
        ui->customPlot->addGraph();
        ui->customPlot->graph(i)->setPen(QPen(color.at(i)));
    }

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);
    ui->customPlot->xAxis->setVisible(true);
    ui->customPlot->xAxis->setLabel(QString("时间"));
    ui->customPlot->yAxis->setLabel(QString("温度"));
//     ui->customPlot->axisRect()->setupFullAxesBox();  //设置四轴可见
}

void FrmTemperatureplot::drawDataPoint(quint8 sensorNum, QVector<float> &mTemp)
{
//    double key = timeStart.msecsTo(QTime::currentTime()) / 1000.0; // time elapsed since start of demo, in seconds
    double key = QTime::currentTime().msecsSinceStartOfDay()*0.001;

    for(quint8 i = 0; i < sensorNum; i++)
    {
        ui->customPlot->graph(i)->addData(key, mTemp.data()[i]);
    }
    // make key axis range scroll with the data (at a constant range size of 8):
//    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);

    ui->customPlot->graph(0)->rescaleValueAxis(false, true);
    for(quint8 i = 1; i < sensorNum; i++)
    {
        ui->customPlot->graph(i)->rescaleValueAxis(true, true);
        ui->customPlot->graph(i)->rescaleKeyAxis();
    }
    ui->customPlot->replot();
}

void FrmTemperatureplot::clearData()
{
    ui->customPlot->graph(0)->data().data()->clear();
    ui->customPlot->graph(1)->data().data()->clear();
}
