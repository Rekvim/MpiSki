#ifndef MPI_H
#define MPI_H

#pragma once
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>

#include "Sensor.h"
#include "./Src/Uart/UartReader.h"
#include "./Src/Mpi/MpiSettings.h"

class Mpi : public QObject
{
    Q_OBJECT

public:
    explicit Mpi(QObject *parent = nullptr);
    ~Mpi();

    bool initialize();
    bool isConnect();
    const QString &portName() const;

    quint8 version();

    quint8 digitalOutputs();
    quint8 digitalInputs();

    void SetDAC_Raw(quint16 value);
    void SetDAC_Real(qreal value);

    quint16 dacMin();
    quint16 dacMax();

    quint8 sensorCount() const;

    Sensor* operator[](quint8 n);

    Sensor* dac() const;

    void setDiscreteOutput(quint8 index, bool state);

public slots:
    void onAdcData(QVector<quint16> adc);
    void onUartConnected(const QString portName);
    void onUartDisconnected();
    void onUartError(QSerialPort::SerialPortError err);

signals:
    void errorOccured(const QString &message);

    void ConnectToUart();

    void GetVersion(quint8 &version);

    void SetDAC(quint16 value);

    void SetChannels(quint8 channels);

    void SetTimer(quint16 timer);

    void TurnADC_On();

    void TurnADC_Off();

    void GetADC(QVector<quint16> &adc);

    void ADC_Timer(bool enable, quint16 interval = 50);

    void setDigitalOutput(quint8 index, bool state);

    void GetDO(quint8 &DO);
    void GetDI(quint8 &DI);

private:
    std::array<Sensor*, 6> m_sensorByAdc { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    UartReader *m_uartReader;
    QThread *m_uartThread;
    bool m_isConnected = false;
    QString m_portName;

    QVector<Sensor *> m_sensors;
    Sensor *m_dac = nullptr;

    quint16 m_dacMin = 65536 * 3 / 24;
    quint16 m_dacMax = 65536 * 21 / 24;

    void sleep(quint16 msecs);
};

#endif // MPI_H
