#ifndef UARTMESSAGE_H
#define UARTMESSAGE_H

#pragma once
#include <QByteArray>

static constexpr char startbyte = 0xAA;

enum class Command : quint8 {
    DAC         = 0xC2,
    ADC         = 0xE1,
    SetChADC    = 0x98,
    ADC_OnOff   = 0x48,
    GetVersion  = 0x00,
    TimerArr    = 0x73,
    SetDO       = 0x41,
    GetDO       = 0x42,
    GetDI       = 0x43,
    OK          = 0x25,
    WrongCRC    = 0x66,
    UnknownCode = 0x13
};

class UartMessage
{
public:
    UartMessage() = default;
    UartMessage(const Command command);
    UartMessage(const Command command, const QByteArray &data);
    UartMessage(const Command command, quint16 data);
    UartMessage(const Command command, quint8 data);
    UartMessage(const QByteArray data);

    QByteArray ToByteArray() const;
    bool CheckCrc() const;
    void AddCrc();
    Command GetCommand() const;
    QByteArray GetData() const;

private:
    QByteArray Crc16() const;
    Command m_command;
    QByteArray m_data;
    QByteArray m_crc;
};

#endif // UARTMESSAGE_H
