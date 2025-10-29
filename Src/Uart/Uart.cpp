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

    connect(m_serialPort, &QSerialPort::errorOccurred,
            this, [this](QSerialPort::SerialPortError error) { emit errorOccurred(error); });
}

Uart::~Uart()
{
    portClosed();
}

void Uart::open(const QString &portName)
{
    if (m_serialPort->isOpen() && m_serialPort->portName() == portName)
        return;

    portClosed();

    m_serialPort->setPortName(portName);
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        emit portOpened(portName);
    }
}

void Uart::close()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portClosed();
    }
}

void Uart::writeAndRead(const QByteArray &dataToWrite, QByteArray &readData)
{
    readData.clear();

    m_serialPort->write(dataToWrite);

    if (!m_serialPort->waitForBytesWritten(10)) {
        return;
    }

    if (m_serialPort->waitForReadyRead(500)) {
        readData.push_back(m_serialPort->readAll());
    }

    while (m_serialPort->waitForReadyRead(10)) {
        readData.push_back(m_serialPort->readAll());
    }
}
