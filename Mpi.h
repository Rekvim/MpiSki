#ifndef MPI_H
#define MPI_H

#include <QObject>

#include "Sensor.h"
#include "UartReader.h"

class MPI : public QObject
{
    Q_OBJECT
public:
    explicit MPI(QObject *parent = nullptr);
    ~MPI();
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
    Sensor *GetDAC();
    void SetDiscreteOutput(quint8 DO_num, bool state);

private:
    UartReader *uart_reader_;
    QThread *uart_thread_;
    bool is_connected_ = false;
    QString port_name_;
    QVector<Sensor *> sensors_;
    Sensor *dac_ = new Sensor;

    void Sleep(quint16 msecs);
    quint16 DAC_MIN = 65536 * 3 / 24;
    quint16 DAC_MAX = 65536 * 21 / 24;
public slots:
    void ADC(QVector<quint16> adc);
    void UART_connected(const QString port_name);
    void UART_disconnected();
    void UART_error(QSerialPort::SerialPortError err);
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
