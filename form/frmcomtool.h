#ifndef FRMCOMTOOL_H
#define FRMCOMTOOL_H

#include <QWidget>
#include "qextserialport.h"

QT_BEGIN_NAMESPACE
namespace Ui { class frmComTool; }
QT_END_NAMESPACE

class frmComTool : public QWidget
{
    Q_OBJECT

public:
    frmComTool(QWidget *parent = nullptr);
    ~frmComTool();

    //处理返回数据
    static bool ParseRS68RetrunData(QByteArray &rs68data, quint8 &slaveaddr, QByteArray &parsedata);

private:
    Ui::frmComTool *ui;

    bool isShow;                //是否显示数据
    bool isSave;                //是否保存
    int sleepTime;              //接收延时时间

    QextSerialPort *com;        //串口通信对象
    bool comOk;                 //串口是否打开

    QTimer *timerRead;          //定时读取串口数据
    QTimer *timerSend;          //定时发送串口数据
    QTimer *timerSave;          //定时保存串口数据

    int sendCount;              //发送数据累计
    int receiveCount;           //接收数据累计

private slots:
    void initForm();            //初始化窗体数据
    void initConfig();          //初始化配置文件
    void saveConfig();          //保存配置文件
    void append(int type, const QString &data, bool clear = false);

    void readData();            //读取串口数据
    void sendData();            //发送串口数据
    void sendData(QString data);//发送串口数据带参数
    void saveData();//保存串口数据

private slots:
    void on_pushButton_OpenCom_clicked();
    void on_pushButton_ReceiveCnt_clicked();
    void on_pushButton_SendCnt_clicked();
    void on_checkBox_AutoSendInTime_stateChanged(int arg1);
    void on_checkBox_SaveInFile_stateChanged(int arg1);
    void on_pushButton_ClearTxtMain_clicked();
    void on_pushButton_ReadTemp_clicked();
};
#endif // FRMCOMTOOL_H
