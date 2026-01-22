#include "Mpi.h"
#include <utility>

Mpi::Mpi(QObject *parent)
    : QObject{parent}
{
    m_dac = new Sensor(this);

    m_uartReader = new UartReader;
    m_uartThread = new QThread(this);

    connect(m_uartThread, &QThread::finished,
            m_uartReader, &QObject::deleteLater);

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

    connect(this, &Mpi::setDigitalOutput,
            m_uartReader, &UartReader::setDigitalOutput,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::GetDO,
            m_uartReader, &UartReader::readDigitalOutputs,
            Qt::BlockingQueuedConnection);

    connect(this, &Mpi::GetDI,
            m_uartReader, &UartReader::readDigitalInputs,
            Qt::BlockingQueuedConnection);

    connect(m_uartReader, &UartReader::adcDataReady,
            this, &Mpi::onAdcData);

    connect(m_uartReader, &UartReader::portOpened,
            this, &Mpi::onUartConnected,
            Qt::QueuedConnection);

    connect(m_uartReader, &UartReader::portClosed,
            this, &Mpi::onUartDisconnected,
            Qt::QueuedConnection);

    connect(m_uartReader, &UartReader::portError,
            this, &Mpi::onUartError,
            Qt::QueuedConnection);

    // connect(m_uartReader, &UartReader::errorOccured,
    //         this, &Mpi::errorOccured,
    //         Qt::DirectConnection);
}

Mpi::~Mpi()
{
    m_uartThread->quit();
    m_uartThread->wait();
}

bool Mpi::isConnect()
{
    emit ConnectToUart();
    return m_isConnected;
}

quint8 Mpi::version()
{
    if (!m_isConnected)
        return 0;
    quint8 version;
    emit GetVersion(version);
    return version;
}

quint8 Mpi::digitalOutputs()
{
    if (!m_isConnected)
        return 0;

    quint8 DO;
    emit GetDO(DO);
    return DO;
}

quint8 Mpi::digitalInputs()
{
    if (!m_isConnected)
        return 0;

    quint8 DI;
    emit GetDI(DI);
    return DI;
}

bool Mpi::initialize()
{
    emit ADC_Timer(false);

    if (!m_isConnected) return false;

    qDeleteAll(m_sensors);
    m_sensors.clear();
    m_sensorByAdc.fill(nullptr);

    const MpiSettings mpiSettings;

    m_dac->setCoefficients(24.0 / 0xFFFF, mpiSettings.GetDac().bias);

    m_dacMin = 65536 * (mpiSettings.GetDac().min - mpiSettings.GetDac().bias) / 24;
    m_dacMax = 65536 * (mpiSettings.GetDac().max - mpiSettings.GetDac().bias) / 24;

    emit SetChannels(0x3F);
    emit TurnADC_On();

    QVector<quint16> adc;
    sleep(1000);
    emit GetADC(adc);

    quint8 sensorNum = 0;
    quint8 channelMask = 0;

    for (const auto& a : std::as_const(adc)) {
        if ((a & 0xFFF) > 0x050) {

            quint8 adcNum = a >> 12;

            channelMask |= (1 << adcNum);

            Sensor *sensor = new Sensor(this);
            m_sensorByAdc[adcNum] = sensor;

            const qreal adcCur = mpiSettings.GetAdc(adcNum);

            const auto sensorSettings = mpiSettings.GetSensor(sensorNum);
            qreal k = ((sensorSettings.max - sensorSettings.min) * adcCur) / (16 * 0xFFF);
            qreal b = (5 * sensorSettings.min - sensorSettings.max) / 4;

            sensor->setCoefficients(k, b);

            sensor->setUnit((sensorNum++ == 0) ? "мм" : "bar");
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
    if (value < m_dacMin)
        value = m_dacMin;
    if (value > m_dacMax)
        value = m_dacMax;
    m_dac->setValue(value);
    emit SetDAC(m_dac->rawValue());
}

void Mpi::SetDAC_Real(qreal value)
{
    quint16 newRaw = m_dac->rawFromValue(value);
    quint16 curRaw = m_dac->rawValue();

    if (newRaw != curRaw) {
        m_dac->setValue(newRaw);
        emit SetDAC(newRaw);
    }
}

quint16 Mpi::dacMin()
{
    return m_dacMin;
}

quint16 Mpi::dacMax()
{
    return m_dacMax;
}

quint8 Mpi::sensorCount() const
{
    return m_sensors.size();
}

const QString &Mpi::portName() const
{
    return m_portName;
}

Sensor *Mpi::operator[](quint8 n)
{
    if (n >= m_sensors.size())
        return nullptr;
    return m_sensors.at(n);
}

Sensor *Mpi::dac() const
{
    return m_dac;
}

void Mpi::setDiscreteOutput(quint8 index, bool state)
{
    if (!m_isConnected)
        return;

    emit setDigitalOutput(index, state);
}

void Mpi::sleep(quint16 msecs)
{
    QEventLoop loop;
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void Mpi::onAdcData(const QVector<quint16>& adc)
{
    // if (m_sensors.size() < adc.size())
    //     return;

    // for (int i = 0; i < adc.size(); ++i) {
    //     m_sensors[i]->setValue(adc.at(i) & 0xFFF);
    // }

    for (quint16 w : adc) {
        const quint8 adcNum = w >> 12;
        const quint16 raw   = w & 0x0FFF;

        if (adcNum < m_sensorByAdc.size() && m_sensorByAdc[adcNum])
            m_sensorByAdc[adcNum]->setValue(raw);
    }
}

void Mpi::onUartConnected(const QString portName)
{
    m_isConnected = true;
    m_portName = portName;
}

void Mpi::onUartDisconnected()
{
    m_isConnected = false;
}

void Mpi::onUartError(QSerialPort::SerialPortError err)
{
    qDebug() << err << Qt::endl;
}
