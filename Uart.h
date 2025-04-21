#ifndef UART_H
#define UART_H

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
    QSerialPort *serial_port_;

public slots:
    void Connect(const QString &port_name);
    void Disconnect();
    void Write_Read(const QByteArray &data_to_write, QByteArray &read_data);
signals:
    void Connected(QString port_name);
    void Disconnected();
    void Error(QSerialPort::SerialPortError err);
};

#endif // UART_H
