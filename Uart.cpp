#include "Uart.h"

Uart::Uart(QObject *parent)
    : QObject{parent}
{
    serial_port_ = new QSerialPort(this);
    serial_port_->setBaudRate(QSerialPort::Baud115200);
    serial_port_->setDataBits(QSerialPort::Data8);
    serial_port_->setParity(QSerialPort::NoParity);
    serial_port_->setStopBits(QSerialPort::OneStop);
    serial_port_->setReadBufferSize(1);

    connect(serial_port_,
            &QSerialPort::errorOccurred,
            this,
            [this](QSerialPort::SerialPortError err) { emit Error(err); });
}

Uart::~Uart()
{
    Disconnect();
}

void Uart::Connect(const QString &port_name)
{
    if (serial_port_->isOpen() && serial_port_->portName() == port_name)
        return;

    Disconnect();

    serial_port_->setPortName(port_name);
    if (serial_port_->open(QSerialPort::ReadWrite)) {
        emit Connected(port_name);
    }
}

void Uart::Disconnect()
{
    if (serial_port_->isOpen()) {
        serial_port_->close();
        emit Disconnected();
    }
}

void Uart::Write_Read(const QByteArray &data_to_write, QByteArray &read_data)
{
    read_data.clear();

    serial_port_->write(data_to_write);

    if (!serial_port_->waitForBytesWritten(10)) {
        return;
    }

    if (serial_port_->waitForReadyRead(500)) {
        read_data.push_back(serial_port_->readAll());
    }

    while (serial_port_->waitForReadyRead(10)) {
        read_data.push_back(serial_port_->readAll());
    }
}
