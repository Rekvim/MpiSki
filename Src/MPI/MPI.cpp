#include "MPI.h"
#include "MpiSettings.h"

MPI::MPI(QObject *parent)
    : QObject{parent}
{
    m_uartReader = new UartReader;
    m_uartThread = new QThread(this);

    m_uartReader->moveToThread(m_uartThread);
    m_uartThread->start();

    connect(this, &MPI::ConnectToUart,
            m_uartReader, &UartReader::autoConnect,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::GetVersion,
            m_uartReader, &UartReader::readVersion,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::SetDAC,
            m_uartReader, &UartReader::setDacValue,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::SetChannels,
            m_uartReader, &UartReader::setAdcChannels,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::SetTimer,
            m_uartReader, &UartReader::setAdcTimerInterval,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::TurnADC_On,
            m_uartReader, &UartReader::enableAdc,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::TurnADC_Off,
            m_uartReader, &UartReader::disableAdc,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::GetADC,
            m_uartReader, &UartReader::readAdcValues,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::ADC_Timer,
            m_uartReader, &UartReader::setAdcPolling,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::SetDO,
            m_uartReader, &UartReader::setDigitalOutput,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::GetDO,
            m_uartReader, &UartReader::readDigitalOutputs,
            Qt::BlockingQueuedConnection);

    connect(this, &MPI::GetDI,
            m_uartReader, &UartReader::readDigitalInputs,
            Qt::BlockingQueuedConnection);

    connect(m_uartReader, &UartReader::adcDataReady,
            this, &MPI::ADC);

    connect(m_uartReader, &UartReader::portOpened,
            this, &MPI::UartConnected,
            Qt::DirectConnection);

    connect(m_uartReader, &UartReader::portClosed,
            this, &MPI::UartDisconnected,
            Qt::DirectConnection);

    connect(m_uartReader, &UartReader::portError,
            this, &MPI::UartError,
            Qt::DirectConnection);

    // connect(m_uartReader, &UartReader::errorOccured,
    //         this, &MPI::errorOccured,
    //         Qt::DirectConnection);
}

MPI::~MPI()
{
    m_uartThread->quit();
    m_uartThread->wait();
}

bool MPI::Connect()
{
    emit ConnectToUart();
    return m_isConnected;
}

quint8 MPI::Version()
{
    if (!m_isConnected)
        return 0;
    quint8 version;
    emit GetVersion(version);
    return version;
}

quint8 MPI::GetDOStatus()
{
    if (!m_isConnected)
        return 0;

    quint8 DO;
    emit GetDO(DO);
    return DO;
}

quint8 MPI::GetDIStatus()
{
    if (!m_isConnected)
        return 0;

    quint8 DI;
    emit GetDI(DI);
    return DI;
}

bool MPI::Initialize()
{
    emit ADC_Timer(false);

    if (!m_isConnected)
        return false;

    const MpiSettings mpiSettings;

    m_dac->SetCoefficients(24.0 / 0xFFFF, mpiSettings.GetDAC().bias);

    DAC_MIN = 65536 * (mpiSettings.GetDAC().min - mpiSettings.GetDAC().bias) / 24;
    DAC_MAX = 65536 * (mpiSettings.GetDAC().max - mpiSettings.GetDAC().bias) / 24;

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
            qreal adcCur = mpiSettings.GetADC(adcNum);
            auto sensorSettings = mpiSettings.GetSensor(sensorNum);
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

void MPI::SetDAC_Raw(quint16 value)
{
    if (value < DAC_MIN)
        value = DAC_MIN;
    if (value > DAC_MAX)
        value = DAC_MAX;
    m_dac->SetValue(value);
    emit SetDAC(m_dac->GetRawValue());
}

void MPI::SetDAC_Real(qreal value)
{
    quint16 dac = m_dac->GetRawFromValue(value);
    if (dac != m_dac->GetRawValue()) {
        m_dac->SetValue(m_dac->GetRawFromValue(value));
        emit SetDAC(m_dac->GetRawValue());
    }
}

quint16 MPI::GetDac_Min()
{
    return DAC_MIN;
}

quint16 MPI::GetDac_Max()
{
    return DAC_MAX;
}

quint8 MPI::SensorCount() const
{
    return m_sensors.size();
}

const QString &MPI::PortName() const
{
    return m_portName;
}

Sensor *MPI::operator[](quint8 n)
{
    if (n >= m_sensors.size())
        return nullptr;
    return m_sensors.at(n);
}

Sensor *MPI::GetDAC()
{
    return m_dac;
}

void MPI::SetDiscreteOutput(quint8 DO_num, bool state)
{
    if (!m_isConnected)
        return;

    emit SetDO(DO_num, state);
}

void MPI::Sleep(quint16 msecs)
{
    QEventLoop loop;
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void MPI::ADC(QVector<quint16> adc)
{
    if (m_sensors.size() < adc.size())
        return;

    for (int i = 0; i < adc.size(); ++i) {
        m_sensors[i]->SetValue(adc.at(i) & 0xFFF);
    }
}

void MPI::UartConnected(const QString portName)
{
    m_isConnected = true;
    m_portName = portName;
}

void MPI::UartDisconnected()
{
    m_isConnected = false;
}

void MPI::UartError(QSerialPort::SerialPortError err)
{
    qDebug() << err << Qt::endl;
}
