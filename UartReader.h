#ifndef UARTREADER_H
#define UARTREADER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include "Uart.h"

const char startbyte = 0xAA;

enum class Command : quint8 {
    DAC = 0xC2,
    ADC = 0xE1,
    SetChADC = 0x98,
    ADC_OnOff = 0x48,
    GetVersion = 0x00,
    TimerArr = 0x73,
    SetDO = 0x41,
    GetDO = 0x42,
    GetDI = 0x43,
    OK = 0x25,
    WrongCRC = 0x66,
    UnknownCode = 0x13
};

class Message
{
public:
    Message() = default;
    Message(const Command command);
    Message(const Command command, const QByteArray data);
    Message(const Command command, quint16 data);
    Message(const Command command, quint8 data);
    Message(const QByteArray data);

    QByteArray ToByteArray() const;
    bool CheckCRC() const;
    void AddCRC();
    Command GetCommand() const;
    QByteArray GetData() const;

private:
    QByteArray CRC16() const;
    Command command_;
    QByteArray data_;
    QByteArray crc_;
};

class UartReader : public QObject
{
    Q_OBJECT
public:
    explicit UartReader(QObject *parent = nullptr);

private:
    Uart *uart_;
    QString port_name_;
    bool is_connected_ = false;
    const quint8 max_attemps_ = 5;
    quint8 version_;
    QTimer *adc_timer_;

    QByteArray SendMessage(const Message &data);

public slots:
    void ConnectToUart();
    void GetVersion(quint8 &version);
    void SetDAC(quint16 value);
    void SetChannels(quint8 channels);
    void SetTimer(quint16 timer);
    void TurnADC_On();
    void TurnADC_Off();
    void GetADC(QVector<quint16> &adc);
    void ADC_Timer(bool enable, quint16 interval = 50);
    void SetDO(quint8 DO_num, bool state);
    void GetDO(quint8 &DO);
    void GetDI(quint8 &DI);
private slots:
    void Connected(QString port_name);
    void Disconnected();
    void Error(QSerialPort::SerialPortError err);
    void SendADC();
signals:
    void Connect(const QString &port_name);
    void Disconnect();
    void Write_Read(const QByteArray &data_to_write, QByteArray &read_data);

    void ADC(QVector<quint16> adc);
    void UART_connected(const QString port_name);
    void UART_disconnected();
    void UART_error(QSerialPort::SerialPortError err);
};

#endif // UARTREADER_H
