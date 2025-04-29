#ifndef MPI_SETTINGS_H
#define MPI_SETTINGS_H

#include <QApplication>
#include <QSettings>
#include <QObject>

class MPI_Settings : public QObject
{
    Q_OBJECT
public:
    explicit MPI_Settings(QObject *parent = nullptr);

    struct MinMax
    {
        qreal min;
        qreal max;
    };

    struct DAC
    {
        qreal min;
        qreal max;
        qreal bias;
    };

    qreal GetADC(quint8 num) const;
    MinMax GetSensor(quint8 num) const;
    DAC GetDAC() const;

private:
    QVector<qreal> m_ADC;
    DAC m_DAC;

    QVector<MinMax> m_sensors;
};

#endif // MPI_SETTINGS_H
