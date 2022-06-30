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

    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setFont(QFont("Helvetica", 9));
    ui->customPlot->legend->setRowSpacing(-3);
    for(quint8 i = 0; i < 6; i++)
    {
        ui->customPlot->addGraph();
        ui->customPlot->graph(i)->setPen(QPen(color.at(i)));
        ui->customPlot->graph(i)->setName(QString("#%1温度").arg(i+1));
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
        ui->customPlot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 10));
        ui->customPlot->graph(i)->addData(key, mTemp.data()[i]);
    }
    // make key axis range scroll with the data (at a constant range size of 8):
//    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);

    ui->customPlot->graph(0)->rescaleAxes();
    // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
    for(quint8 i = 1; i < sensorNum; i++) {
        ui->customPlot->graph(i)->rescaleAxes(true);
    }
    // Note: we could have also just called customPlot->rescaleAxes(); instead
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

//    ui->customPlot->graph(0)->rescaleValueAxis(false, true);
//    ui->customPlot->graph(0)->rescaleKeyAxis();
//    for(quint8 i = 1; i < sensorNum; i++)
//    {
//        ui->customPlot->graph(i)->rescaleValueAxis(true, true);
//        ui->customPlot->graph(i)->rescaleKeyAxis();
//    }
    ui->customPlot->replot();
}

void FrmTemperatureplot::clearData()
{
    for(quint8 i = 0; i < 6; i++)
    {
        ui->customPlot->graph(i)->data().data()->clear();
    }
    ui->customPlot->replot();
}
