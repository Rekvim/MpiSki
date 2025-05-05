#ifndef UART_H
#define UART_H

#pragma once
#include <QByteArray>
#include <QObject>
#include <QSerialPort>

class Uart : public QObject
{
    Q_OBJECT
public:
    explicit Uart(QObject *parent = nullptr);
    ~Uart();

private:
    QSerialPort *m_serialPort;

public slots:
    void Connect(const QString &portName);
    void Disconnect();
    void Write_Read(const QByteArray &dataToWrite, QByteArray &readData);
signals:
    void Connected(QString portName);
    void Disconnected();
    void Error(QSerialPort::SerialPortError err);
};

#endif // UART_H
