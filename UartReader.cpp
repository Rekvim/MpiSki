#include <QDebug>
#include <QSerialPortInfo>

#include "UartReader.h"
UartReader::UartReader(QObject *parent)
    : QObject{parent}
{
    m_uart = new Uart(this);

    m_adcTimer = new QTimer(this);

    connect(m_uart, &Uart::Connected, this, &UartReader::Connected, Qt::DirectConnection);
    connect(m_uart, &Uart::Disconnected, this, &UartReader::Disconnected, Qt::DirectConnection);
    connect(m_uart, &Uart::Error, this, &UartReader::Error, Qt::DirectConnection);

    connect(this, &UartReader::Connect, m_uart, &Uart::Connect, Qt::DirectConnection);
    connect(this, &UartReader::Disconnect, m_uart, &Uart::Disconnect, Qt::DirectConnection);
    connect(this, &UartReader::Write_Read, m_uart, &Uart::Write_Read, Qt::DirectConnection);

    connect(m_adcTimer, &QTimer::timeout, this, &UartReader::SendADC);
}

QByteArray UartReader::SendMessage(const Message &data)
{
    for (quint8 attempt = 0; attempt < m_maxAttemps; ++attempt) {
        QByteArray readData;
        emit Write_Read(data.ToByteArray(), readData);
        Message message(readData);
        if (message.CheckCRC() && message.GetCommand() == Command::OK)
            return message.GetData();
        else
            qDebug() << m_portName << data.ToByteArray() << Qt::endl;
    }
    return QByteArray();
}

void UartReader::ConnectToUart()
{
    foreach (QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        emit Connect(port.portName());
        QByteArray version = SendMessage({Command::GetVersion});
        if (!version.isEmpty()) {
            m_version = version.at(0);
            return;
        }
        emit Disconnect();
    }
}

void UartReader::GetVersion(quint8 &version)
{
    if (!m_isConnected)
        return;
    version = m_version;
}

void UartReader::SetDAC(quint16 value)
{
    if (!m_isConnected)
        return;
    SendMessage({Command::DAC, quint16(value)});
}

void UartReader::SetChannels(quint8 channels)
{
    if (!m_isConnected)
        return;
    SendMessage({Command::SetChADC, channels});
}

void UartReader::SetTimer(quint16 timer)
{
    if (!m_isConnected)
        return;
    SendMessage({Command::TimerArr, timer});
}

void UartReader::TurnADC_On()
{
    if (!m_isConnected)
        return;
    SendMessage({Command::ADC_OnOff, quint8(1)});
}

void UartReader::TurnADC_Off()
{
    if (!m_isConnected)
        return;
    SendMessage({Command::ADC_OnOff, quint8(0)});
}

void UartReader::GetADC(QVector<quint16> &adc)
{
    if (!m_isConnected)
        return;
    QByteArray data = SendMessage({Command::ADC});

    adc.clear();
    for (int i = 0; i < data.size(); i += 2) {
        adc.push_back(((data.at(i) & 0xFF) << 8) + (data.at(i + 1) & 0xFF));
    }
}

void UartReader::ADC_Timer(bool enable, quint16 interval)
{
    if (m_isConnected && enable)
        m_adcTimer->start(interval);
    else
        m_adcTimer->stop();
}

void UartReader::SetDO(quint8 DO_num, bool state)
{
    if (!m_isConnected)
        return;

    if (DO_num > 7)
        return;

    quint8 value = (state ? 0x08 : 0) | DO_num;
    SendMessage({Command::SetDO, value});
}

void UartReader::GetDO(quint8 &DO)
{
    if (!m_isConnected)
        return;

    QByteArray message = SendMessage({Command::GetDO});

    if (!message.isEmpty()) {
        DO = message.at(0);
    }
}

void UartReader::GetDI(quint8 &DI)
{
    if (!m_isConnected)
        return;

    QByteArray message = SendMessage({Command::GetDI});

    if (!message.isEmpty()) {
        DI = message.at(0);
    }
}

void UartReader::Connected(QString port_name)
{
    m_portName = port_name;
    m_isConnected = true;
    emit UartConnected(port_name);
}

void UartReader::Disconnected()
{
    m_isConnected = false;
    m_adcTimer->stop();
    emit UartDisconnected();
}

void UartReader::Error(QSerialPort::SerialPortError err)
{
    if (err != QSerialPort::NoError && err != QSerialPort::TimeoutError)
        emit UartError(err);
}

void UartReader::SendADC()
{
    QVector<quint16> result;
    GetADC(result);
    emit ADC(result);
}

Message::Message(const Command command, const QByteArray &data)
    : m_command(command)
    , m_data(data)
{}

Message::Message(const Command command)
    : m_command(command)
{}

Message::Message(const Command command, quint16 data)
    : m_command(command)
{
    m_data.push_back(data >> 8);
    m_data.push_back(data);
}

Message::Message(const Command command, quint8 data)
    : m_command(command)
{
    m_data.push_back(data);
}

Message::Message(const QByteArray data)
{
    if (data.size() < 5)
        return;
    if (data.at(0) != startbyte)
        return;
    quint8 len = data.at(1);
    if (len + 4 != data.size())
        return;

    switch (data.at(2)) {
    case static_cast<quint8>(Command::OK):
        m_command = Command::OK;
        break;
    case static_cast<quint8>(Command::UnknownCode):
        m_command = Command::UnknownCode;
        break;
    case static_cast<quint8>(Command::WrongCRC):
        m_command = Command::WrongCRC;
        break;
    default:
        return;
    }

    for (int i = 1; i < len; ++i) {
        m_data.push_back(data.at(i + 2));
    }

    m_crc.push_back(data.at(len + 2));
    m_crc.push_back(data.at(len + 3));
}

QByteArray Message::ToByteArray() const
{
    QByteArray result;
    result.push_back(startbyte);
    result.push_back(m_data.length() + 1);
    result.push_back(static_cast<quint8>(m_command));
    result.push_back(m_data);
    if (m_crc.isEmpty())
        result.push_back(CRC16());
    result.push_back(m_crc);
    return result;
}

bool Message::CheckCRC() const
{
    return CRC16() == m_crc;
}

void Message::AddCRC()
{
    m_crc = CRC16();
}

Command Message::GetCommand() const
{
    return m_command;
}

QByteArray Message::GetData() const
{
    return m_data;
}

QByteArray Message::CRC16() const
{
    quint16 wCrc = 0xFFFF;

    wCrc ^= static_cast<quint8>(startbyte) << 8;
    for (int i = 0; i < 8; i++) {
        wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
    }

    wCrc ^= static_cast<quint8>(m_data.length() + 1) << 8;
    for (int i = 0; i < 8; i++) {
        wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
    }

    wCrc ^= static_cast<quint8>(m_command) << 8;
    for (int i = 0; i < 8; i++) {
        wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
    }

    foreach (quint8 n, m_data) {
        wCrc ^= n << 8;

        for (int i = 0; i < 8; i++) {
            wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
        }
    }

    QByteArray result;
    result.push_back(wCrc >> 8);
    result.push_back(wCrc);

    return result;
}
