#pragma once
#include <QObject>
#include <QTimer>
#include <QSerialPortInfo>
#include <QDebug>

#include "Uart.h"
#include "UartMessage.h"

class UartReader : public QObject
{
    Q_OBJECT
public:
    explicit UartReader(QObject *parent = nullptr);

private:
    QByteArray SendMessage(const UartMessage &message);

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
    void Connected(const QString &portName);
    void Disconnected();
    void Error(QSerialPort::SerialPortError err);
    void SendADC();

signals:
    void Connect(const QString &portName);
    void Disconnect();
    void Write_Read(const QByteArray &data_to_write, QByteArray &read_data);

    void ADC(QVector<quint16> adc);
    void UartConnected(const QString &portName);
    void UartDisconnected();
    void UartError(QSerialPort::SerialPortError err);

private:
    Uart *m_uart;
    QString m_portName;
    bool m_isConnected = false;
    const quint8 m_maxAttempts = 5;
    quint8 m_version;
    QTimer *m_adcTimer;
};
