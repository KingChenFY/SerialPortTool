#include "frmcomtool.h"
#include "ui_frmcomtool.h"

#include "quihelper.h"
#include "quihelperdata.h"

#include <QDebug>

frmComTool::frmComTool(QWidget *parent) : QWidget(parent), ui(new Ui::frmComTool)
{
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
    QUIHelper::setFormInCenter(this);
}

frmComTool::~frmComTool()
{
    delete ui;
}

void frmComTool::initForm()
{
    isShow = true;
    isSave = false;
    sleepTime = 10;
    sendCount = 0;
    receiveCount = 0;

    //主动关联的槽
    connect(ui->pushButton_Send, SIGNAL(clicked()), this, SLOT(sendData()));

    //定时读取数据
    timerRead = new QTimer(this);
    timerRead->setInterval(100);
    connect(timerRead, SIGNAL(timeout()), this, SLOT(readData()));

    //发送数据
    timerSend = new QTimer(this);
    connect(timerSend, SIGNAL(timeout()), this, SLOT(sendData()));

    //保存数据
    timerSave = new QTimer(this);
    connect(timerSave, SIGNAL(timeout()), this, SLOT(saveData()));
}

void frmComTool::initConfig()
{
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    qDebug() << "portName: " <<portList.at(0).portName();
    qDebug() << "portDescription: " <<portList.at(0).description();

    QStringList comList;
    for (int i = 1; i <= 20; i++) {
        comList << QString("COM%1").arg(i);
    }
    comList << "ttyUSB0" << "ttyS0" << "ttyS1" << "ttyS2" << "ttyS3" << "ttyS4";
    comList << "ttymxc1" << "ttymxc2" << "ttymxc3" << "ttymxc4";
    comList << "ttySAC1" << "ttySAC2" << "ttySAC3" << "ttySAC4";
    ui->ComboBox_ComNumber->addItems(comList);
    ui->ComboBox_ComNumber->setCurrentIndex(ui->ComboBox_ComNumber->findText(AppConfig::PortName));
    connect(ui->ComboBox_ComNumber, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList baudList;
    baudList << "50" << "75" << "100" << "134" << "150" << "200" << "300" << "600" << "1200"
             << "1800" << "2400" << "4800" << "9600" << "14400" << "19200" << "38400"
             << "56000" << "57600" << "76800" << "115200" << "128000" << "256000";
    ui->ComboBox_BaudRate->addItems(baudList);
    ui->ComboBox_BaudRate->setCurrentIndex(ui->ComboBox_BaudRate->findText(QString::number(AppConfig::BaudRate)));
    connect(ui->ComboBox_BaudRate, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList dataBitsList;
    dataBitsList << "5" << "6" << "7" << "8";
    ui->ComboBox_DataBit->addItems(dataBitsList);
    ui->ComboBox_DataBit->setCurrentIndex(ui->ComboBox_DataBit->findText(QString::number(AppConfig::DataBit)));
    connect(ui->ComboBox_DataBit, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList parityList;
    parityList << "无" << "奇" << "偶";
#ifdef Q_OS_WIN
    parityList << "标志";
#endif
    parityList << "空格";
    ui->ComboBox_Parity->addItems(parityList);
    ui->ComboBox_Parity->setCurrentIndex(ui->ComboBox_Parity->findText(AppConfig::Parity));
    connect(ui->ComboBox_Parity, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList stopBitsList;
    stopBitsList << "1";
#ifdef Q_OS_WIN
    stopBitsList << "1.5";
#endif
    stopBitsList << "2";
    ui->ComboBox_StopBit->addItems(stopBitsList);
    ui->ComboBox_StopBit->setCurrentIndex(ui->ComboBox_StopBit->findText(QString::number(AppConfig::StopBit)));
    connect(ui->ComboBox_StopBit, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->lineEdit_AutoSendInTime->setText(QString::number(AppConfig::SendInterval));
    connect(ui->lineEdit_AutoSendInTime, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    timerSend->setInterval(AppConfig::SendInterval);
}

void frmComTool::saveConfig()
{
    AppConfig::PortName = ui->ComboBox_ComNumber->currentText();
    AppConfig::BaudRate = ui->ComboBox_BaudRate->currentText().toInt();
    AppConfig::DataBit = ui->ComboBox_DataBit->currentText().toInt();
    AppConfig::Parity = ui->ComboBox_Parity->currentText();
    AppConfig::StopBit = ui->ComboBox_StopBit->currentText().toDouble();

    int sendInterval = ui->lineEdit_AutoSendInTime->text().toInt();
    if (sendInterval != AppConfig::SendInterval) {
        AppConfig::SendInterval = sendInterval;
        timerSend->setInterval(AppConfig::SendInterval);
    }

    AppConfig::writeConfig();
}

void frmComTool::append(int type, const QString &data, bool clear)
{
    static int currentCount = 0;
    static int maxCount = 100;

    if (clear) {
        ui->txtMain->clear();
        currentCount = 0;
        return;
    }

    if (currentCount >= maxCount) {
        ui->txtMain->clear();
        currentCount = 0;
    }

    if (!isShow) {
        return;
    }

    //过滤回车换行符
    QString strData = data;
    strData = strData.replace("\r", "");
    strData = strData.replace("\n", "");

    //不同类型不同颜色显示
    QString strType;
    if (type == 0) {
        strType = "串口发送 >>";
        ui->txtMain->setTextColor(QColor("dodgerblue"));
    } else if (type == 1) {
        strType = "串口接收 <<";
        ui->txtMain->setTextColor(QColor("red"));
    } else if (type == 2) {
        strType = "处理延时 >>";
        ui->txtMain->setTextColor(QColor("gray"));
    } else if (type == 3) {
        strType = "正在校验 >>";
        ui->txtMain->setTextColor(QColor("green"));
    } else if (type == 4) {
        strType = "网络发送 >>";
        ui->txtMain->setTextColor(QColor(24, 189, 155));
    } else if (type == 5) {
        strType = "网络接收 <<";
        ui->txtMain->setTextColor(QColor(255, 107, 107));
    } else if (type == 6) {
        strType = "提示信息 >>";
        ui->txtMain->setTextColor(QColor(100, 184, 255));
    }

    strData = QString("时间[%1] %2 %3").arg(TIMEMS).arg(strType).arg(strData);
    ui->txtMain->append(strData);
    currentCount++;
}

void frmComTool::readData()
{
    if (com->bytesAvailable() <= 0) {
        return;
    }

    QUIHelper::sleep(sleepTime);
    QByteArray data = com->readAll();
    int dataLen = data.length();
    if (dataLen <= 0) {
        return;
    }

    if (isShow) {
        QString buffer;
        if (ui->checkBox_HexReceive->isChecked()) {
            buffer = QUIHelperData::byteArrayToHexStr(data);
        } else {
            //buffer = QUIHelperData::byteArrayToAsciiStr(data);
            buffer = QString::fromLocal8Bit(data);
        }

        //启用调试则模拟调试数据
//        if (ui->ckDebug->isChecked()) {
//            int count = AppData::Keys.count();
//            for (int i = 0; i < count; i++) {
//                if (buffer.startsWith(AppData::Keys.at(i))) {
//                    sendData(AppData::Values.at(i));
//                    break;
//                }
//            }
//        }

        append(1, buffer);
        receiveCount = receiveCount + data.size();
        ui->pushButton_ReceiveCnt->setText(QString("接收：%1 字节").arg(receiveCount));

        if(isSave) {
            QString fileName = ui->lineEdit_SaveDir->text();
            QFile file(fileName);
            file.open(QFile::WriteOnly | QIODevice::Append);
            QTextStream out(&file);
//            buffer = buffer.replace("\r", "");
//            buffer = buffer.replace("\n", "");
            QString strData = QString("T[%1],%2").arg(TIMEMS).arg(buffer);
            out << strData << Qt::endl;
            file.close();
        }

        //启用网络转发则调用网络发送数据
//        if (tcpOk) {
//            socket->write(data);
//            append(4, QString(buffer));
//        }
    }
}

void frmComTool::sendData()
{
    QString str = ui->comboBox_SendContent->currentText();
    if (str.isEmpty()) {
        ui->comboBox_SendContent->setFocus();
        return;
    }

    sendData(str);

//    if (ui->ckAutoClear->isChecked()) {
//        ui->cboxData->setCurrentIndex(-1);
//        ui->cboxData->setFocus();
//    }
}

void frmComTool::sendData(QString data)
{
    if (com == 0 || !com->isOpen()) {
        return;
    }

//    //AT硬件调试
//    if (data.startsWith("AT")) {
//        data += "\r";
//    }

    QByteArray buffer;
    if (ui->checkBox_HexSend->isChecked()) {
        buffer = QUIHelperData::hexStrToByteArray(data);
    } else {
        buffer = QUIHelperData::asciiStrToByteArray(data);
    }

    com->write(buffer);
    append(0, data);
    sendCount = sendCount + buffer.size();
    ui->pushButton_SendCnt->setText(QString("发送：%1 字节").arg(sendCount));
}

void frmComTool::saveData()
{
    QString tempData = ui->txtMain->toPlainText();

    if (tempData == "") {
        return;
    }

    if (ui->lineEdit_SaveDir->text().isEmpty()) {
        return;
    }

//    QDateTime now = QDateTime::currentDateTime();
//    QString name = now.toString("yyyy-MM-dd-HH-mm-ss");
//    QString fileName = QString("%1/%2.txt").arg(ui->lineEdit_SaveDir->text()).arg(name);

    QString fileName = ui->lineEdit_SaveDir->text();
    QFile file(fileName);
    file.open(QFile::WriteOnly | QIODevice::Text | QIODevice::Append);
    QTextStream out(&file);
    out << tempData;
    file.close();

    on_pushButton_ClearTxtMain_clicked();
}

void frmComTool::on_pushButton_OpenCom_clicked()
{
    if (ui->pushButton_OpenCom->text() == "打开串口") {
        com = new QextSerialPort(ui->ComboBox_ComNumber->currentText(), QextSerialPort::Polling);
        comOk = com->open(QIODevice::ReadWrite);

        if (comOk) {
            //清空缓冲区
            com->flush();
            //设置波特率
            com->setBaudRate((BaudRateType)ui->ComboBox_BaudRate->currentText().toInt());
            //设置数据位
            com->setDataBits((DataBitsType)ui->ComboBox_DataBit->currentText().toInt());
            //设置校验位
            com->setParity((ParityType)ui->ComboBox_Parity->currentIndex());
            //设置停止位
            com->setStopBits((StopBitsType)ui->ComboBox_StopBit->currentIndex());
            com->setFlowControl(FLOW_OFF);
            com->setTimeout(10);

            //changeEnable(true);
            ui->pushButton_OpenCom->setText("关闭串口");
            timerRead->start();
        }
    } else {
        timerRead->stop();
        com->close();
        com->deleteLater();

        //changeEnable(false);
        ui->pushButton_OpenCom->setText("打开串口");
        //on_btnClear_clicked();
        comOk = false;
        ui->checkBox_AutoSendInTime->setChecked(false);
        emit on_checkBox_AutoSendInTime_stateChanged(0);
    }
}

void frmComTool::on_pushButton_ReceiveCnt_clicked()
{
    receiveCount = 0;
    ui->pushButton_ReceiveCnt->setText("接收：0 字节");
}

void frmComTool::on_pushButton_SendCnt_clicked()
{
    sendCount = 0;
    ui->pushButton_SendCnt->setText("发送：0 字节");
}

void frmComTool::on_checkBox_AutoSendInTime_stateChanged(int arg1)
{
    if (arg1 == 0) {
        timerSend->stop();
        ui->lineEdit_AutoSendInTime->setEnabled(true);
        ui->comboBox_SendContent->setEnabled(true);
    } else {
        ui->lineEdit_AutoSendInTime->setEnabled(false);
        ui->comboBox_SendContent->setEnabled(false);
        timerSend->start();
    }
}

void frmComTool::on_checkBox_SaveInFile_stateChanged(int arg1)
{
    if(0 == arg1)
    {
        isSave = false;
    }
    else
    {
        QString dir = QFileDialog::getSaveFileName(this, "Save File","",tr("Text files (*.txt)"));
    //    QString dir = QFileDialog::getSaveFileName(this, "Save File","",tr("Text files (*.txt);;XML files (*.xml)"));
        if (!dir.isEmpty()) {
            ui->lineEdit_SaveDir->setText(dir);
            isSave = true;
        }
    }
}


void frmComTool::on_pushButton_ClearTxtMain_clicked()
{
    append(0, "", true);
}

