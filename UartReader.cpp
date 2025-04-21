#include <QDebug>
#include <QSerialPortInfo>

#include "UartReader.h"
UartReader::UartReader(QObject *parent)
    : QObject{parent}
{
    uart_ = new Uart(this);

    adc_timer_ = new QTimer(this);

    connect(uart_, &Uart::Connected, this, &UartReader::Connected, Qt::DirectConnection);
    connect(uart_, &Uart::Disconnected, this, &UartReader::Disconnected, Qt::DirectConnection);
    connect(uart_, &Uart::Error, this, &UartReader::Error, Qt::DirectConnection);

    connect(this, &UartReader::Connect, uart_, &Uart::Connect, Qt::DirectConnection);
    connect(this, &UartReader::Disconnect, uart_, &Uart::Disconnect, Qt::DirectConnection);
    connect(this, &UartReader::Write_Read, uart_, &Uart::Write_Read, Qt::DirectConnection);

    connect(adc_timer_, &QTimer::timeout, this, &UartReader::SendADC);
}

QByteArray UartReader::SendMessage(const Message &data)
{
    for (quint8 attempt = 0; attempt < max_attemps_; ++attempt) {
        QByteArray read_data;
        emit Write_Read(data.ToByteArray(), read_data);
        Message message(read_data);
        if (message.CheckCRC() && message.GetCommand() == Command::OK)
            return message.GetData();
        else
            qDebug() << port_name_ << data.ToByteArray() << Qt::endl;
    }
    return QByteArray();
}

void UartReader::ConnectToUart()
{
    foreach (QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        emit Connect(port.portName());
        QByteArray version = SendMessage({Command::GetVersion});
        if (!version.isEmpty()) {
            version_ = version.at(0);
            return;
        }
        emit Disconnect();
    }
}

void UartReader::GetVersion(quint8 &version)
{
    if (!is_connected_)
        return;
    version = version_;
}

void UartReader::SetDAC(quint16 value)
{
    if (!is_connected_)
        return;
    SendMessage({Command::DAC, quint16(value)});
}

void UartReader::SetChannels(quint8 channels)
{
    if (!is_connected_)
        return;
    SendMessage({Command::SetChADC, channels});
}

void UartReader::SetTimer(quint16 timer)
{
    if (!is_connected_)
        return;
    SendMessage({Command::TimerArr, timer});
}

void UartReader::TurnADC_On()
{
    if (!is_connected_)
        return;
    SendMessage({Command::ADC_OnOff, quint8(1)});
}

void UartReader::TurnADC_Off()
{
    if (!is_connected_)
        return;
    SendMessage({Command::ADC_OnOff, quint8(0)});
}

void UartReader::GetADC(QVector<quint16> &adc)
{
    if (!is_connected_)
        return;
    QByteArray data = SendMessage({Command::ADC});

    adc.clear();
    for (int i = 0; i < data.size(); i += 2) {
        adc.push_back(((data.at(i) & 0xFF) << 8) + (data.at(i + 1) & 0xFF));
    }
}

void UartReader::ADC_Timer(bool enable, quint16 interval)
{
    if (is_connected_ && enable)
        adc_timer_->start(interval);
    else
        adc_timer_->stop();
}

void UartReader::SetDO(quint8 DO_num, bool state)
{
    if (!is_connected_)
        return;

    if (DO_num > 7)
        return;

    quint8 value = (state ? 0x08 : 0) | DO_num;
    SendMessage({Command::SetDO, value});
}

void UartReader::GetDO(quint8 &DO)
{
    if (!is_connected_)
        return;

    QByteArray message = SendMessage({Command::GetDO});

    if (!message.isEmpty()) {
        DO = message.at(0);
    }
}

void UartReader::GetDI(quint8 &DI)
{
    if (!is_connected_)
        return;

    QByteArray message = SendMessage({Command::GetDI});

    if (!message.isEmpty()) {
        DI = message.at(0);
    }
}

void UartReader::Connected(QString port_name)
{
    port_name_ = port_name;
    is_connected_ = true;
    emit UART_connected(port_name);
}

void UartReader::Disconnected()
{
    is_connected_ = false;
    adc_timer_->stop();
    emit UART_disconnected();
}

void UartReader::Error(QSerialPort::SerialPortError err)
{
    if (err != QSerialPort::NoError && err != QSerialPort::TimeoutError)
        emit UART_error(err);
}

void UartReader::SendADC()
{
    QVector<quint16> result;
    GetADC(result);
    emit ADC(result);
}

Message::Message(const Command command, const QByteArray data)
    : command_(command)
    , data_(data)
{}

Message::Message(const Command command)
    : command_(command)
{}

Message::Message(const Command command, quint16 data)
    : command_(command)
{
    data_.push_back(data >> 8);
    data_.push_back(data);
}

Message::Message(const Command command, quint8 data)
    : command_(command)
{
    data_.push_back(data);
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
        command_ = Command::OK;
        break;
    case static_cast<quint8>(Command::UnknownCode):
        command_ = Command::UnknownCode;
        break;
    case static_cast<quint8>(Command::WrongCRC):
        command_ = Command::WrongCRC;
        break;
    default:
        return;
    }

    for (int i = 1; i < len; ++i) {
        data_.push_back(data.at(i + 2));
    }

    crc_.push_back(data.at(len + 2));
    crc_.push_back(data.at(len + 3));
}

QByteArray Message::ToByteArray() const
{
    QByteArray result;
    result.push_back(startbyte);
    result.push_back(data_.length() + 1);
    result.push_back(static_cast<quint8>(command_));
    result.push_back(data_);
    if (crc_.isEmpty())
        result.push_back(CRC16());
    result.push_back(crc_);
    return result;
}

bool Message::CheckCRC() const
{
    return CRC16() == crc_;
}

void Message::AddCRC()
{
    crc_ = CRC16();
}

Command Message::GetCommand() const
{
    return command_;
}

QByteArray Message::GetData() const
{
    return data_;
}

QByteArray Message::CRC16() const
{
    quint16 wCrc = 0xFFFF;

    wCrc ^= static_cast<quint8>(startbyte) << 8;
    for (int i = 0; i < 8; i++) {
        wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
    }

    wCrc ^= static_cast<quint8>(data_.length() + 1) << 8;
    for (int i = 0; i < 8; i++) {
        wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
    }

    wCrc ^= static_cast<quint8>(command_) << 8;
    for (int i = 0; i < 8; i++) {
        wCrc = (wCrc & 0x8000) ? (wCrc << 1) ^ 0x1021 : (wCrc << 1);
    }

    foreach (quint8 n, data_) {
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
