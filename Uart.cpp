#include "Uart.h"

Uart::Uart(QObject *parent)
    : QObject{parent}
{
    m_serialPort = new QSerialPort(this);
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setReadBufferSize(1);

    connect(m_serialPort,
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
    if (m_serialPort->isOpen() && m_serialPort->portName() == port_name)
        return;

    Disconnect();

    m_serialPort->setPortName(port_name);
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        emit Connected(port_name);
    }
}

void Uart::Disconnect()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit Disconnected();
    }
}

void Uart::Write_Read(const QByteArray &data_to_write, QByteArray &read_data)
{
    read_data.clear();

    m_serialPort->write(data_to_write);

    if (!m_serialPort->waitForBytesWritten(10)) {
        return;
    }

    if (m_serialPort->waitForReadyRead(500)) {
        read_data.push_back(m_serialPort->readAll());
    }

    while (m_serialPort->waitForReadyRead(10)) {
        read_data.push_back(m_serialPort->readAll());
    }
}
