#include "UartMessage.h"

UartMessage::UartMessage(const Command command)
    : m_command(command)
{}

UartMessage::UartMessage(const Command command, quint8 data)
    : m_command(command)
{
    m_data.push_back(data);
}

UartMessage::UartMessage(const Command command, quint16 data)
    : m_command(command)
{
    m_data.push_back(data >> 8);
    m_data.push_back(data);
}

UartMessage::UartMessage(const Command command, const QByteArray &data)
    : m_command(command)
    , m_data(data)
{}

UartMessage::UartMessage(const QByteArray data)
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

QByteArray UartMessage::ToByteArray() const
{
    QByteArray result;
    result.push_back(startbyte);
    result.push_back(m_data.length() + 1);
    result.push_back(static_cast<quint8>(m_command));
    result.push_back(m_data);
    if (m_crc.isEmpty())
        result.push_back(Crc16());
    result.push_back(m_crc);
    return result;
}

bool UartMessage::CheckCrc() const
{
    return Crc16() == m_crc;
}

void UartMessage::AddCrc()
{
    m_crc = Crc16();
}

Command UartMessage::GetCommand() const
{
    return m_command;
}

QByteArray UartMessage::GetData() const
{
    return m_data;
}

QByteArray UartMessage::Crc16() const
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
