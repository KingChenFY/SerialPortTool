#include "serialportcombobox.h"

serialportcombobox::serialportcombobox(QWidget *parent)
    : QComboBox{parent}
{

}

serialportcombobox::~serialportcombobox()
{

}

void serialportcombobox::showPopup()
{
    QString currentComName = QComboBox::currentText();
    QComboBox::clear();

    QList<QSerialPortInfo> comList = QSerialPortInfo::availablePorts();
    QStringList comNameList;
    quint8 comAvaliableNum = comList.size();
    if(0 != comAvaliableNum) {
        for (int i = 0; i < comList.size(); i++) {
//            comNameList << comList.at(i).portName() + ' ' + comList.at(i).description();
            comNameList << comList.at(i).portName();
        }
        QComboBox::addItems(comNameList);
        QComboBox::setCurrentIndex(this->findText(currentComName));
    }
    QComboBox::showPopup();
}

void serialportcombobox::hidePopup()
{
    QComboBox::hidePopup();
}
