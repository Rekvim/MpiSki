#ifndef MPI_H
#define MPI_H

#pragma once
#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>

#include "Sensor.h"
#include "./src/Uart/UartReader.h"

class MPI : public QObject
{
    Q_OBJECT
public:
    explicit MPI(QObject *parent = nullptr);
    ~MPI();

    bool Initialize();
    bool Connect();
    quint8 Version();

    quint8 GetDOStatus();
    quint8 GetDIStatus();

    void SetDAC_Raw(quint16 value);
    void SetDAC_Real(qreal value);

    Sensor *GetDAC() const;
    quint16 GetDac_Min() const;
    quint16 GetDac_Max() const;

    quint8 SensorCount() const;
    const QString &PortName() const;
    Sensor *operator[](quint8 n) const;
    void SetDiscreteOutput(quint8 DO_num, bool state);

private:
    bool m_isConnected = false;

    UartReader *m_uartReader;
    QThread *m_uartThread;
    QString m_portName;
    QVector<Sensor *> m_sensors;
    Sensor *m_dac = new Sensor;

    quint16 m_DAC_MIN = 65536 * 3 / 24;
    quint16 m_DAC_MAX = 65536 * 21 / 24;

    void Sleep(quint16 msecs);

public slots:
    void ADC(QVector<quint16> adc);
    void UartConnected(const QString portName);
    void UartDisconnected();
    void UartError(QSerialPort::SerialPortError err);
signals:
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
