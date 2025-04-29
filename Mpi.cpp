#include "Mpi.h"
#include "MpiSettings.h"

MPI::MPI(QObject *parent)
    : QObject{parent}
{
    m_uartReader = new UartReader;
    m_uartThread = new QThread(this);
    m_uartReader->moveToThread(m_uartThread);
    m_uartThread->start();

    connect(this,
            &MPI::ConnectToUart,
            m_uartReader,
            &UartReader::ConnectToUart,
            Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::GetVersion,
            m_uartReader,
            &UartReader::GetVersion,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::SetDAC, m_uartReader, &UartReader::SetDAC, Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::SetChannels,
            m_uartReader,
            &UartReader::SetChannels,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::SetTimer, m_uartReader, &UartReader::SetTimer, Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::TurnADC_On,
            m_uartReader,
            &UartReader::TurnADC_On,
            Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::TurnADC_Off,
            m_uartReader,
            &UartReader::TurnADC_Off,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::GetADC, m_uartReader, &UartReader::GetADC, Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::ADC_Timer,
            m_uartReader,
            &UartReader::ADC_Timer,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::SetDO, m_uartReader, &UartReader::SetDO, Qt::BlockingQueuedConnection);
    connect(this, &MPI::GetDO, m_uartReader, &UartReader::GetDO, Qt::BlockingQueuedConnection);
    connect(this, &MPI::GetDI, m_uartReader, &UartReader::GetDI, Qt::BlockingQueuedConnection);

    connect(m_uartReader, &UartReader::ADC, this, &MPI::ADC);
    connect(m_uartReader,
            &UartReader::UartConnected,
            this,
            &MPI::UART_connected,
            Qt::DirectConnection);
    connect(m_uartReader,
            &UartReader::UartDisconnected,
            this,
            &MPI::UART_disconnected,
            Qt::DirectConnection);
    connect(m_uartReader, &UartReader::UartError, this, &MPI::UART_error, Qt::DirectConnection);
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

    const MPI_Settings mpi_settings;

    m_dac->SetCoefficients(24.0 / 0xFFFF, mpi_settings.GetDAC().bias);

    DAC_MIN = 65536 * (mpi_settings.GetDAC().min - mpi_settings.GetDAC().bias) / 24;
    DAC_MAX = 65536 * (mpi_settings.GetDAC().max - mpi_settings.GetDAC().bias) / 24;

    for (const auto sensor : m_sensors) {
        delete sensor;
    }
    m_sensors.clear();

    emit SetChannels(0x3F);
    emit TurnADC_On();
    QVector<quint16> adc;

    Sleep(1000);

    emit GetADC(adc);

    quint8 sensor_num = 0;
    quint8 channel_mask = 0;
    for (const auto &a : adc) {
        if ((a & 0xFFF) > 0x050) {
            quint8 adc_num = a >> 12;

            channel_mask |= (1 << adc_num);

            Sensor *sensor = new Sensor;
            qreal adc_cur = mpi_settings.GetADC(adc_num);
            auto sensor_settings = mpi_settings.GetSensor(sensor_num);
            qreal k = ((sensor_settings.max - sensor_settings.min) * adc_cur) / (16 * 0xFFF);
            qreal b = (5 * sensor_settings.min - sensor_settings.max) / 4;
            sensor->SetCoefficients(k, b);
            if (sensor_num++ == 0) {
                sensor->SetUnit("мм");
            } else {
                sensor->SetUnit("bar");
            }
            m_sensors.push_back(sensor);
        }
    }

    if (sensor_num == 0) {
        emit TurnADC_Off();
        return true;
    }

    emit SetChannels(channel_mask);
    emit SetTimer(40 / sensor_num);
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

void MPI::UART_connected(const QString port_name)
{
    m_isConnected = true;
    m_portName = port_name;
}

void MPI::UART_disconnected()
{
    m_isConnected = false;
}

void MPI::UART_error(QSerialPort::SerialPortError err)
{
    qDebug() << err << Qt::endl;
}
