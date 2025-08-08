// UartMessage.cpp
#include "UartMessage.h"

// --- Конструкторы для формирования фрейма ---

UartMessage::UartMessage(Command command)
    : m_command(command)
{}

UartMessage::UartMessage(Command command, quint8 data)
    : m_command(command)
{
    m_data.push_back(data);
}

UartMessage::UartMessage(Command command, quint16 data)
    : m_command(command)
{
    m_data.push_back(static_cast<char>(data >> 8));
    m_data.push_back(static_cast<char>(data & 0xFF));
}

UartMessage::UartMessage(Command command, const QByteArray &data)
    : m_command(command)
    , m_data(data)
{}

// --- Конструктор разбора принятого фрейма ---
UartMessage::UartMessage(const QByteArray &data)
{
    // Минимальный размер: startbyte(1) + len(1) + cmd(1) + CRC(2) = 5
    if (data.size() < 5) return;
    // Проверяем стартовый байт
    if (static_cast<quint8>(data.at(0)) != static_cast<quint8>(startbyte))
        return;
    // Длина поля length = len(payload)+1
    quint8 len = static_cast<quint8>(data.at(1));
    // Полный размер должен быть len+4
    if (len + 4 != static_cast<quint8>(data.size()))
        return;

    // 1) Сохраняем код команды (любой)
    quint8 cmd = static_cast<quint8>(data.at(2));
    m_command = static_cast<Command>(cmd);

    // 2) Копируем payload (len-1 байтов после команды)
    //    payload начинается с data[3], длина = len-1
    for (int i = 0; i < len - 1; ++i) {
        m_data.push_back(data.at(3 + i));
    }

    // 3) Копируем CRC (2 байта сразу после payload)
    m_crc.push_back(data.at(3 + (len - 1)));
    m_crc.push_back(data.at(3 + (len - 1) + 1));
}

// --- Преобразование в raw-байты для отправки ---
QByteArray UartMessage::ToByteArray() const
{
    QByteArray result;
    result.push_back(startbyte);
    // длина = payload.size() + 1 (байт команды)
    result.push_back(static_cast<char>(m_data.length() + 1));
    result.push_back(static_cast<char>(static_cast<quint8>(m_command)));
    // payload
    result.append(m_data);
    // CRC (если ещё не добавлено — добавляем автоматически)
    if (m_crc.isEmpty()) {
        QByteArray crc = Crc16();
        result.append(crc);
    } else {
        result.append(m_crc);
    }
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

// --- Реализация CRC-16-CCITT (poly=0x1021, init=0xFFFF) ---
QByteArray UartMessage::Crc16() const
{
    quint16 crc = 0xFFFF;

    // Helper для одного байта
    auto processByte = [&](quint8 b){
        crc ^= quint16(b) << 8;
        for (int i = 0; i < 8; ++i) {
            crc = (crc & 0x8000)
            ? (crc << 1) ^ 0x1021
            : (crc << 1);
        }
    };

    // 1) стартовый байт
    processByte(static_cast<quint8>(startbyte));
    // 2) длина
    processByte(static_cast<quint8>(m_data.length() + 1));
    // 3) команда
    processByte(static_cast<quint8>(m_command));
    // 4) payload
    for (auto byte : m_data) {
        processByte(static_cast<quint8>(byte));
    }

    QByteArray out;
    out.push_back(static_cast<char>(crc >> 8));
    out.push_back(static_cast<char>(crc & 0xFF));
    return out;
}
