#include "UartReader.h"

UartReader::UartReader(QObject *parent)
    : QObject(parent)
{
    m_uart = new Uart(this);
    m_adcPollTimer = new QTimer(this);

    connect(m_uart, &Uart::portOpened,
            this, &UartReader::onPortOpened,
            Qt::DirectConnection);

    connect(m_uart, &Uart::portClosed,
            this, &UartReader::onPortClosed,
            Qt::DirectConnection);

    connect(m_uart, &Uart::errorOccurred,
            this, &UartReader::onPortError,
            Qt::DirectConnection);

    connect(this, &UartReader::openPort,
            m_uart, &Uart::open,
            Qt::DirectConnection);

    connect(this, &UartReader::closePort,
            m_uart, &Uart::close,
            Qt::DirectConnection);

    connect(this, &UartReader::writeAndRead,
            m_uart, &Uart::writeAndRead,
            Qt::DirectConnection);

    connect(m_adcPollTimer, &QTimer::timeout,
            this, &UartReader::onAdcPollTimer);
}

QByteArray UartReader::SendMessage(const UartMessage &message)
{
    for (quint8 attempt = 0; attempt < m_maxAttempts; ++attempt) {
        QByteArray readData;
        emit writeAndRead(message.ToByteArray(), readData);

        if (readData.isEmpty()) {
        } else {
            UartMessage response(readData);

            if (response.CheckCrc()
                && (response.GetCommand() == Command::OK
                    || response.GetCommand() == message.GetCommand()))
            {
                return response.GetData();
            }
        }
    }
    return QByteArray();
}

void UartReader::autoConnect()
{
    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        emit openPort(port.portName());
        QByteArray version = SendMessage(UartMessage(Command::GetVersion));
        if (!version.isEmpty()) {
            m_version = version.at(0);
            return;
        }
        emit closePort();
    }
}
void UartReader::readVersion(quint8 &version)
{
    if (!m_isConnected) return;
    version = m_version;
}

void UartReader::setDacValue(quint16 value)
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::DAC, value));
}

void UartReader::setAdcChannels(quint8 channels)
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::SetChADC, channels));
}

void UartReader::setAdcTimerInterval(quint16 timer)
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::TimerArr, timer));
}

void UartReader::enableAdc()
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::ADC_OnOff, static_cast<quint8>(1)));
}

void UartReader::disableAdc()
{
    if (!m_isConnected) return;
    SendMessage(UartMessage(Command::ADC_OnOff, static_cast<quint8>(0)));
}

void UartReader::readAdcValues(QVector<quint16> &adc)
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

void UartReader::setAdcPolling(bool enable, quint16 interval)
{
    if (m_isConnected && enable)
        m_adcPollTimer->start(interval);
    else
        m_adcPollTimer->stop();
}

void UartReader::setDigitalOutput(quint8 outputNumber, bool state)
{
    if (!m_isConnected || outputNumber > 7) return;
    quint8 value = (state ? 0x08 : 0) | outputNumber;
    SendMessage(UartMessage(Command::SetDO, value));
}

void UartReader::readDigitalOutputs(quint8 &digitalOutputs)
{
    if (!m_isConnected) return;
    QByteArray raw = SendMessage(UartMessage(Command::GetDO));
    if (!raw.isEmpty())
        digitalOutputs = static_cast<quint8>(raw.at(0));
}

void UartReader::readDigitalInputs(quint8 &digitalInputs)
{
    if (!m_isConnected) return;
    QByteArray raw = SendMessage(UartMessage(Command::GetDI));
    if (!raw.isEmpty())
        digitalInputs = static_cast<quint8>(raw.at(0));
}

void UartReader::onPortOpened(const QString &portName)
{
    m_portName = portName;
    m_isConnected = true;
    emit portOpened(portName);
}

void UartReader::onPortClosed()
{
    m_isConnected = false;
    m_adcPollTimer->stop();
    emit portClosed();
}

void UartReader::onPortError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError)
        emit portError(error);
}

void UartReader::onAdcPollTimer()
{
    QVector<quint16> data;
    readAdcValues(data);
    emit adcDataReady(data);
}
