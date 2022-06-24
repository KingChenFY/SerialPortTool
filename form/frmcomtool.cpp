#include "frmcomtool.h"
#include "ui_frmcomtool.h"
#include <QMessageBox>
#include <QDebug>

#include "quihelper.h"
#include "quihelperdata.h"

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
    regAddr = 0;
    sensornum = 1;

    //主动关联的槽
    connect(ui->pushButton_Send, SIGNAL(clicked()), this, SLOT(sendData()));

    //定时读取数据
    timerRead = new QTimer(this);
    timerRead->setInterval(100);
    connect(timerRead, SIGNAL(timeout()), this, SLOT(readData()));

    //发送数据
    timerSend = new QTimer(this);
    //connect(timerSend, SIGNAL(timeout()), this, SLOT(sendData()));
    connect(timerSend, SIGNAL(timeout()), this, SLOT(on_pushButton_ReadTemp_clicked()));

    //保存数据
//    timerSave = new QTimer(this);
//    connect(timerSave, SIGNAL(timeout()), this, SLOT(saveData()));
}

void frmComTool::initConfig()
{
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

    QStringList rsbaudList;
    rsbaudList << "300" << "600" << "1200" << "2400" << "4800" << "9600" << "19200"
               << "38400" << "43000" << "56000" << "57600" << "115200";
    ui->comboBox_SlaveBaudRate->addItems(rsbaudList);
    ui->comboBox_SlaveBaudRate->setCurrentIndex(ui->comboBox_SlaveBaudRate->findText(QString::number(9600)));

    ui->comboBox_SlaveParity->addItems(parityList);
    ui->comboBox_SlaveParity->setCurrentIndex(ui->comboBox_SlaveParity->findText("无"));

    ui->comboBox_SlaveStopBits->addItems(stopBitsList);
    ui->comboBox_SlaveStopBits->setCurrentIndex(ui->comboBox_SlaveStopBits->findText(QString::number(1)));

    QStringList sensorList;
    sensorList << "PT100" << "PT1000";
    ui->comboBox_SlaveSensorType->addItems(sensorList);
    ui->comboBox_SlaveSensorType->setCurrentIndex(ui->comboBox_SlaveSensorType->findText("PT100"));
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

void frmComTool::RsModuleAppend(ushort regaddr, ushort bytenum, QByteArray prasedata)
{
    ushort regnum =bytenum/2;
    int data[64] = {0};
    float tValue1, tValue2;
    QString sValue;

    for (int i = 0; i < regnum; i += 1) {
        data[i] = QUIHelperData::byteToUShort(prasedata.sliced(i*2, 2));
    }
    ui->SpinBox_SlaveAddrNum->setValue(regaddr);

    tValue1 = data[0] / 100.0;
    tValue2 = data[1] / 100.0;
    if(0 == regAddr)
    {
        if(sensornum < ui->spinBox_pt02num->value())
        {
            if(1 == sensornum)
            {
                ui->lineEdit_TempValue_1->setText(QString("%1").arg(tValue1));
                ui->lineEdit_TempValue_2->setText(QString("%1").arg(tValue2));
                 sValue = QString("1, %1, 2, %2").arg(tValue1).arg(tValue2);
            }
            else
            {
                ui->lineEdit_TempValue_3->setText(QString("%1").arg(tValue1));
                ui->lineEdit_TempValue_4->setText(QString("%1").arg(tValue2));
                sValue = QString("3, %1, 4, %2").arg(tValue1).arg(tValue2);
            }

            if(isSave) {
                saveData(sValue);
            }
            sensornum++;
            ui->SpinBox_SendAddr->setValue(sensornum);
            on_pushButton_ReadTemp_clicked();
        }
        else
        {
            if(2 == sensornum)
            {
                ui->lineEdit_TempValue_3->setText(QString("%1").arg(tValue1));
                ui->lineEdit_TempValue_4->setText(QString("%1").arg(tValue2));
                sValue = QString("3, %1, 4, %2").arg(tValue1).arg(tValue2);
            }
            else
            {
                ui->lineEdit_TempValue_5->setText(QString("%1").arg(tValue1));
                ui->lineEdit_TempValue_6->setText(QString("%1").arg(tValue2));
                sValue = QString("5, %1, 6, %2").arg(tValue1).arg(tValue2);
            }
            if(isSave) {
                saveData(sValue);
            }
            sensornum = 1;
            ui->SpinBox_SendAddr->setValue(sensornum);
        }
    }
    else if(1 == regAddr)
    {
        ui->SpinBox_SlaveAddrNum->setValue(data[0]);
    }
    else if(3 == regAddr)
    {
        ui->comboBox_SlaveBaudRate->setCurrentIndex(data[0]);
    }
    else if(9 == regAddr)
    {
        ui->comboBox_SlaveParity->setCurrentIndex(data[0]);
    }
    else if(8 == regAddr)
    {
        ui->comboBox_SlaveStopBits->setCurrentIndex(data[0]);
    }
    else if(7 == regAddr)
    {
        ui->comboBox_SlaveSensorType->setCurrentIndex(data[0]);
    }
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
        QByteArray __parsedata;
        quint8 __slaveaddr;
        quint8 __regnum;

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

//        if(isSave) {
//            saveData(buffer);
//        }

        append(1, buffer);
        receiveCount = receiveCount + data.size();
        ui->pushButton_ReceiveCnt->setText(QString("接收：%1 字节").arg(receiveCount));

        if(QUIHelperData::ParseRS68RetrunData(data, __slaveaddr, __regnum, __parsedata))
        {
            RsModuleAppend(__slaveaddr, __regnum, __parsedata);
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
    if(!comOk) return;
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

void frmComTool::saveData(QString &tempData)
{
//    QString tempData = ui->txtMain->toPlainText();
//    if (tempData == "") {
//        return;
//    }
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
    QString strData = QString("T[%1],%2").arg(TIMEMS).arg(tempData);
    out << strData << Qt::endl;
    file.close();

//    on_pushButton_ClearTxtMain_clicked();
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
        ui->tab_RS68Module->setEnabled(true);
    } else {
        if(ui->lineEdit_AutoSendInTime->text().toUInt() < 1000)
        {
            QMessageBox::warning(this, tr("Warning"), tr("发送间隔需要大于1秒！"), QMessageBox::Abort);
            ui->checkBox_AutoSendInTime->setChecked(false);
            ui->checkBox_AutoSendInTime->setCheckState(Qt::Unchecked);
            return;
        }
//        else if(ui->comboBox_SendContent->currentText().isEmpty())
//        {
//            QMessageBox::information(this, tr("Hint"), tr("发送内容为空！"), QMessageBox::Ok);
//            ui->checkBox_AutoSendInTime->setChecked(false);
//            ui->checkBox_AutoSendInTime->setCheckState(Qt::Unchecked);
//            return;
//        }
        else
        {
            ui->tab_RS68Module->setEnabled(false);
            ui->lineEdit_AutoSendInTime->setEnabled(false);
            ui->comboBox_SendContent->setEnabled(false);
            timerSend->start();
        }
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


void frmComTool::on_pushButton_ReadTemp_clicked()
{
    QByteArray __read_buffer;

    regAddr = 0;
    if(1 == ui->SpinBox_SendAddr->value())
    {
        ui->lineEdit_TempValue_1->clear();
        ui->lineEdit_TempValue_2->clear();
    }
    else if(2 == ui->SpinBox_SendAddr->value())
    {
        ui->lineEdit_TempValue_3->clear();
        ui->lineEdit_TempValue_4->clear();
    }
    else
    {
        ui->lineEdit_TempValue_5->clear();
        ui->lineEdit_TempValue_6->clear();
    }
    QUIHelperData::FormatRS68SendData(ui->SpinBox_SendAddr->value(), 0x04, regAddr, 2, __read_buffer);
    sendData(QUIHelperData::byteArrayToHexStr(__read_buffer));
}


void frmComTool::on_pushButton_SlaveRead_clicked()
{
    QByteArray __read_buffer;

    if(ui->radioButton_SlaveAddrNum->isChecked())
    {
        regAddr = 1;
    }
    else if(ui->radioButton_SlaveBaudRate->isChecked())
    {
        regAddr = 3;
    }
    else if(ui->radioButton_SlaveParity->isChecked())
    {
        regAddr = 9;
    }
    else if(ui->radioButton_SlaveStopBits->isChecked())
    {
        regAddr = 8;
    }
    else
    {
        regAddr = 7;
    }
    QUIHelperData::FormatRS68SendData(ui->SpinBox_SendAddr->value(), 0x03, regAddr, 1, __read_buffer);
    sendData(QUIHelperData::byteArrayToHexStr(__read_buffer));
}


void frmComTool::on_pushButton_SlaveSet_clicked()
{
    QByteArray __write_buffer;
    ushort data;

    if(ui->radioButton_SlaveAddrNum->isChecked())
    {
        regAddr = 1;
        data = ui->SpinBox_SlaveAddrNum->value();
    }
    else if(ui->radioButton_SlaveBaudRate->isChecked())
    {
        regAddr = 3;
        data = ui->comboBox_SlaveBaudRate->currentIndex();
    }
    else if(ui->radioButton_SlaveParity->isChecked())
    {
        regAddr = 9;
        data = ui->comboBox_SlaveParity->currentIndex();
    }
    else if(ui->radioButton_SlaveStopBits->isChecked())
    {
        regAddr = 8;
        data = ui->comboBox_SlaveStopBits->currentIndex();
    }
    else
    {
        regAddr = 7;
        data = ui->comboBox_SlaveSensorType->currentIndex();
    }
    QUIHelperData::FormatRS68SendData(ui->SpinBox_SendAddr->value(), 0x06, regAddr, data, __write_buffer);
    sendData(QUIHelperData::byteArrayToHexStr(__write_buffer));
}


void frmComTool::on_pushButton_ResetRsModule_clicked()
{
    QByteArray __write_buffer, __d_buffer;
    ushort data;

    regAddr = 37;

    __d_buffer.clear();
    __d_buffer.append(0x55);
    __d_buffer.append(0xAA); ;
    data = QUIHelperData::byteToUShort(__d_buffer);

    QUIHelperData::FormatRS68SendData(ui->SpinBox_SendAddr->value(), 0x06, regAddr, data, __write_buffer);
    sendData(QUIHelperData::byteArrayToHexStr(__write_buffer));

    emit on_pushButton_OpenCom_clicked();
}

void frmComTool::on_pushButton_ResetConnectParam_clicked()
{
    QByteArray __write_buffer, __d_buffer;
    ushort data;

    regAddr = 265;

    __d_buffer.clear();
    __d_buffer.append(0x12);
    __d_buffer.append(0x34); ;
    data = QUIHelperData::byteToUShort(__d_buffer);

    QUIHelperData::FormatRS68SendData(ui->SpinBox_SendAddr->value(), 0x06, regAddr, data, __write_buffer);
    sendData(QUIHelperData::byteArrayToHexStr(__write_buffer));
}

