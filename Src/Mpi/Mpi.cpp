#include "Mpi.h"
#include "MpiSettings.h"

Mpi::Mpi(QObject *parent)
    : QObject{parent}
{
    m_uartReader = new UartReader;
    m_uartThread = new QThread(this);

    m_uartReader->moveToThread(m_uartThread);
    m_uartThread->start();

    connect(this, &Mpi::ConnectToUart,
            m_uartReader, &UartReader::autoConnect,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::GetVersion,
            m_uartReader, &UartReader::readVersion,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::SetDAC,
            m_uartReader, &UartReader::setDacValue,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::SetChannels,
            m_uartReader, &UartReader::setAdcChannels,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::SetTimer,
            m_uartReader, &UartReader::setAdcTimerInterval,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::TurnADC_On,
            m_uartReader, &UartReader::enableAdc,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::TurnADC_Off,
            m_uartReader, &UartReader::disableAdc,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::GetADC,
            m_uartReader, &UartReader::readAdcValues,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::ADC_Timer,
            m_uartReader, &UartReader::setAdcPolling,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::SetDO,
            m_uartReader, &UartReader::setDigitalOutput,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::GetDO,
            m_uartReader, &UartReader::readDigitalOutputs,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::GetDI,
            m_uartReader, &UartReader::readDigitalInputs,
            Qt::BlockingQueuedConnection);

    connect(m_uartReader, &UartReader::adcDataReady,
            this, &Mpi::ADC);

    connect(m_uartReader, &UartReader::portOpened,
            this, &Mpi::UartConnected,
            Qt::DirectConnection);

    connect(m_uartReader, &UartReader::portClosed,
            this, &Mpi::UartDisconnected,
            Qt::DirectConnection);

    connect(m_uartReader, &UartReader::portError,
            this, &Mpi::UartError,
            Qt::DirectConnection);

    // connect(m_uartReader, &UartReader::errorOccured,
    //         this, &Mpi::errorOccured,
    //         Qt::DirectConnection);
}

Mpi::~Mpi()
{
    m_uartThread->quit();
    m_uartThread->wait();
}

bool Mpi::Connect()
{
    emit ConnectToUart();
    return m_isConnected;
}

quint8 Mpi::Version()
{
    if (!m_isConnected)
        return 0;
    quint8 version;
    emit GetVersion(version);
    return version;
}

quint8 Mpi::GetDOStatus()
{
    if (!m_isConnected)
        return 0;

    quint8 DO;
    emit GetDO(DO);
    return DO;
}

quint8 Mpi::GetDIStatus()
{
    if (!m_isConnected)
        return 0;

    quint8 DI;
    emit GetDI(DI);
    return DI;
}

bool Mpi::Initialize()
{
    emit ADC_Timer(false);

    if (!m_isConnected)
        return false;

    const MpiSettings MpiSettings;

    m_dac->SetCoefficients(24.0 / 0xFFFF, MpiSettings.GetDac().bias);

    dacMin = 65536 * (MpiSettings.GetDac().min - MpiSettings.GetDac().bias) / 24;
    dacMax = 65536 * (MpiSettings.GetDac().max - MpiSettings.GetDac().bias) / 24;

    for (const auto &sensor : m_sensors) {
        delete sensor;
    }
    m_sensors.clear();

    emit SetChannels(0x3F);
    emit TurnADC_On();
    QVector<quint16> adc;

    Sleep(1000);

    emit GetADC(adc);

    quint8 sensorNum = 0;
    quint8 channelMask = 0;
    for (const auto &a : adc) {
        if ((a & 0xFFF) > 0x050) {
            quint8 adcNum = a >> 12;

            channelMask |= (1 << adcNum);

            Sensor *sensor = new Sensor;
            qreal adcCur = MpiSettings.GetAdc(adcNum);
            auto sensorSettings = MpiSettings.GetSensor(sensorNum);
            qreal k = ((sensorSettings.max - sensorSettings.min) * adcCur) / (16 * 0xFFF);
            qreal b = (5 * sensorSettings.min - sensorSettings.max) / 4;
            sensor->SetCoefficients(k, b);
            if (sensorNum++ == 0) {
                sensor->SetUnit("мм");
            } else {
                sensor->SetUnit("bar");
            }
            m_sensors.push_back(sensor);
        }
    }

    if (sensorNum == 0) {
        emit TurnADC_Off();
        return true;
    }

    emit SetChannels(channelMask);
    emit SetTimer(40 / sensorNum);
    emit ADC_Timer(true, 50);
    return true;
}

void Mpi::SetDAC_Raw(quint16 value)
{
    if (value < dacMin)
        value = dacMin;
    if (value > dacMax)
        value = dacMax;
    m_dac->SetValue(value);
    emit SetDAC(m_dac->GetRawValue());
}

void Mpi::SetDAC_Real(qreal value)
{
    quint16 dac = m_dac->GetRawFromValue(value);
    if (dac != m_dac->GetRawValue()) {
        m_dac->SetValue(m_dac->GetRawFromValue(value));
        emit SetDAC(m_dac->GetRawValue());
    }
}

quint16 Mpi::GetDac_Min()
{
    return dacMin;
}

quint16 Mpi::GetDac_Max()
{
    return dacMax;
}

quint8 Mpi::SensorCount() const
{
    return m_sensors.size();
}

const QString &Mpi::PortName() const
{
    return m_portName;
}

Sensor *Mpi::operator[](quint8 n)
{
    if (n >= m_sensors.size())
        return nullptr;
    return m_sensors.at(n);
}

Sensor *Mpi::GetDac()
{
    return m_dac;
}

void Mpi::SetDiscreteOutput(quint8 DO_num, bool state)
{
    if (!m_isConnected)
        return;

    emit SetDO(DO_num, state);
}

void Mpi::Sleep(quint16 msecs)
{
    QEventLoop loop;
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void Mpi::ADC(QVector<quint16> adc)
{
    if (m_sensors.size() < adc.size())
        return;

    for (int i = 0; i < adc.size(); ++i) {
        m_sensors[i]->SetValue(adc.at(i) & 0xFFF);
    }
}

void Mpi::UartConnected(const QString portName)
{
    m_isConnected = true;
    m_portName = portName;
}

void Mpi::UartDisconnected()
{
    m_isConnected = false;
}

void Mpi::UartError(QSerialPort::SerialPortError err)
{
    qDebug() << err << Qt::endl;
}
