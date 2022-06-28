#ifndef SERIALPORTCOMBOBOX_H
#define SERIALPORTCOMBOBOX_H

#include <QComboBox>
#include <QSerialPort>
#include <QSerialPortInfo>

class serialportcombobox : public QComboBox
{
    Q_OBJECT
public:
    explicit serialportcombobox(QWidget *parent = nullptr);
    ~serialportcombobox() override;

    void showPopup() override;
    void hidePopup() override;

signals:

};

#endif // SERIALPORTCOMBOBOX_H
