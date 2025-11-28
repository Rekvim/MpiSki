#ifndef MPI_H
#define MPI_H

#pragma once
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>

#include "Sensor.h"
#include "./Src/Uart/UartReader.h"

class Mpi : public QObject
{
    Q_OBJECT
public:
    explicit Mpi(QObject *parent = nullptr);
    ~Mpi();
    bool Connect();
    quint8 Version();
    quint8 GetDOStatus();
    quint8 GetDIStatus();

    bool Initialize();
    void SetDAC_Raw(quint16 value);
    void SetDAC_Real(qreal value);
    quint16 GetDac_Min();
    quint16 GetDac_Max();
    quint8 SensorCount() const;
    const QString &PortName() const;
    Sensor *operator[](quint8 n);
    Sensor *GetDac();
    void SetDiscreteOutput(quint8 DO_num, bool state);

private:
    UartReader *m_uartReader;
    QThread *m_uartThread;
    bool m_isConnected = false;
    QString m_portName;
    QVector<Sensor *> m_sensors;
    Sensor *m_dac = new Sensor;

    quint16 m_dacMin = 65536 * 3 / 24;
    quint16 m_dacMax = 65536 * 21 / 24;

    void Sleep(quint16 msecs);

public slots:
    void ADC(QVector<quint16> adc);
    void UartConnected(const QString portName);
    void UartDisconnected();
    void UartError(QSerialPort::SerialPortError err);
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
    void SetDO(quint8 DO_num, bool state);
    void GetDO(quint8 &DO);
    void GetDI(quint8 &DI);
};

#endif // MPI_H
