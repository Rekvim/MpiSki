#include "UartReader.h"

UartReader::UartReader(QObject *parent)
    : QObject(parent)
{
    m_uart = new Uart(this);
    m_adcTimer = new QTimer(this);

    connect(m_uart, &Uart::Connected,
            this, &UartReader::Connected,
            Qt::DirectConnection);

    connect(m_uart, &Uart::Disconnected,
            this, &UartReader::Disconnected,
            Qt::DirectConnection);

    connect(m_uart, &Uart::Error,
            this, &UartReader::Error,
            Qt::DirectConnection);

    connect(this, &UartReader::Connect,
            m_uart, &Uart::Connect,
            Qt::DirectConnection);

    connect(this, &UartReader::Disconnect,
            m_uart, &Uart::Disconnect,
            Qt::DirectConnection);

    connect(this, &UartReader::Write_Read,
            m_uart, &Uart::Write_Read,
            Qt::DirectConnection);

    connect(m_adcTimer, &QTimer::timeout,
            this, &UartReader::SendADC);
}

QByteArray UartReader::SendMessage(const UartMessage &message)
{
    for (quint8 attempt = 0; attempt < m_maxAttempts; ++attempt) {
        QByteArray readData;
        emit Write_Read(message.ToByteArray(), readData);
        UartMessage response(readData);
        if (response.CheckCrc() && response.GetCommand() == Command::OK)
            return response.GetData();
        else
            qDebug() << m_portName << message.ToByteArray() << Qt::endl;
    }
    return QByteArray();
}

void UartReader::ConnectToUart()
{
    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        emit Connect(port.portName());
        QByteArray version = SendMessage(UartMessage(Command::GetVersion));
        if (!version.isEmpty()) {
            m_version = version.at(0);
            return;
        }
        emit Disconnect();
    }
}
void UartReader::GetVersion(quint8 &version)
{
    if (!m_isConnected) return;
    version = m_version;
}

void UartReader::SetDAC(quint16 value)
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::DAC, value));
}

void UartReader::SetChannels(quint8 channels)
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::SetChADC, channels));
}

void UartReader::SetTimer(quint16 timer)
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::TimerArr, timer));
}

void UartReader::TurnADC_On()
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::ADC_OnOff, static_cast<quint8>(1)));
}

void UartReader::TurnADC_Off()
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::ADC_OnOff, static_cast<quint8>(0)));
}

void UartReader::GetADC(QVector<quint16> &adc)
{
    if (!m_isConnected) return;
    QByteArray raw = SendMessage(UartMessage(Command::ADC));
    adc.clear();
    for (int i = 0; i + 1 < raw.size(); i += 2) {
        quint16 val = (static_cast<quint8>(raw.at(i)) << 8) |
                      static_cast<quint8>(raw.at(i+1));
        adc.append(val);
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
    if (!m_isConnected || DO_num > 7) return;
    quint8 value = (state ? 0x08 : 0) | DO_num;
    SendMessage(UartMessage(Command::SetDO, value));
}

void UartReader::GetDO(quint8 &DO)
{
    if (!m_isConnected) return;
    QByteArray raw = SendMessage(UartMessage(Command::GetDO));
    if (!raw.isEmpty())
        DO = static_cast<quint8>(raw.at(0));
}

void UartReader::GetDI(quint8 &DI)
{
    if (!m_isConnected) return;
    QByteArray raw = SendMessage(UartMessage(Command::GetDI));
    if (!raw.isEmpty())
        DI = static_cast<quint8>(raw.at(0));
}

void UartReader::Connected(const QString &portName)
{
    m_portName = portName;
    m_isConnected = true;
    emit UartConnected(portName);
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
    QVector<quint16> data;
    GetADC(data);
    emit ADC(data);
}
