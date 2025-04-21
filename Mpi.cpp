#include <QDebug>
#include <QEventLoop>
#include "Mpi.h"
#include "MpiSettings.h"

MPI::MPI(QObject *parent)
    : QObject{parent}
{
    uart_reader_ = new UartReader;
    uart_thread_ = new QThread(this);
    uart_reader_->moveToThread(uart_thread_);
    uart_thread_->start();

    connect(this,
            &MPI::ConnectToUart,
            uart_reader_,
            &UartReader::ConnectToUart,
            Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::GetVersion,
            uart_reader_,
            &UartReader::GetVersion,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::SetDAC, uart_reader_, &UartReader::SetDAC, Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::SetChannels,
            uart_reader_,
            &UartReader::SetChannels,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::SetTimer, uart_reader_, &UartReader::SetTimer, Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::TurnADC_On,
            uart_reader_,
            &UartReader::TurnADC_On,
            Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::TurnADC_Off,
            uart_reader_,
            &UartReader::TurnADC_Off,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::GetADC, uart_reader_, &UartReader::GetADC, Qt::BlockingQueuedConnection);
    connect(this,
            &MPI::ADC_Timer,
            uart_reader_,
            &UartReader::ADC_Timer,
            Qt::BlockingQueuedConnection);
    connect(this, &MPI::SetDO, uart_reader_, &UartReader::SetDO, Qt::BlockingQueuedConnection);
    connect(this, &MPI::GetDO, uart_reader_, &UartReader::GetDO, Qt::BlockingQueuedConnection);
    connect(this, &MPI::GetDI, uart_reader_, &UartReader::GetDI, Qt::BlockingQueuedConnection);

    connect(uart_reader_, &UartReader::ADC, this, &MPI::ADC);
    connect(uart_reader_,
            &UartReader::UART_connected,
            this,
            &MPI::UART_connected,
            Qt::DirectConnection);
    connect(uart_reader_,
            &UartReader::UART_disconnected,
            this,
            &MPI::UART_disconnected,
            Qt::DirectConnection);
    connect(uart_reader_, &UartReader::UART_error, this, &MPI::UART_error, Qt::DirectConnection);
}

MPI::~MPI()
{
    uart_thread_->quit();
    uart_thread_->wait();
}

bool MPI::Connect()
{
    emit ConnectToUart();
    return is_connected_;
}

quint8 MPI::Version()
{
    if (!is_connected_)
        return 0;
    quint8 version;
    emit GetVersion(version);
    return version;
}

quint8 MPI::GetDOStatus()
{
    if (!is_connected_)
        return 0;

    quint8 DO;
    emit GetDO(DO);
    return DO;
}

quint8 MPI::GetDIStatus()
{
    if (!is_connected_)
        return 0;

    quint8 DI;
    emit GetDI(DI);
    return DI;
}

bool MPI::Initialize()
{
    emit ADC_Timer(false);

    if (!is_connected_)
        return false;

    const MPI_Settings mpi_settings;

    dac_->SetCoefficients(24.0 / 0xFFFF, mpi_settings.GetDAC().bias);

    DAC_MIN = 65536 * (mpi_settings.GetDAC().min - mpi_settings.GetDAC().bias) / 24;
    DAC_MAX = 65536 * (mpi_settings.GetDAC().max - mpi_settings.GetDAC().bias) / 24;

    for (const auto sensor : sensors_) {
        delete sensor;
    }
    sensors_.clear();

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
            sensors_.push_back(sensor);
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
    dac_->SetValue(value);
    emit SetDAC(dac_->GetRawValue());
}

void MPI::SetDAC_Real(qreal value)
{
    quint16 dac = dac_->GetRawFromValue(value);
    if (dac != dac_->GetRawValue()) {
        dac_->SetValue(dac_->GetRawFromValue(value));
        emit SetDAC(dac_->GetRawValue());
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
    return sensors_.size();
}

const QString &MPI::PortName() const
{
    return port_name_;
}

Sensor *MPI::operator[](quint8 n)
{
    if (n >= sensors_.size())
        return nullptr;
    return sensors_.at(n);
}

Sensor *MPI::GetDAC()
{
    return dac_;
}

void MPI::SetDiscreteOutput(quint8 DO_num, bool state)
{
    if (!is_connected_)
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
    if (sensors_.size() < adc.size())
        return;

    for (int i = 0; i < adc.size(); ++i) {
        sensors_[i]->SetValue(adc.at(i) & 0xFFF);
    }
}

void MPI::UART_connected(const QString port_name)
{
    is_connected_ = true;
    port_name_ = port_name;
}

void MPI::UART_disconnected()
{
    is_connected_ = false;
}

void MPI::UART_error(QSerialPort::SerialPortError err)
{
    qDebug() << err << Qt::endl;
}
