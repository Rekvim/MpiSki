#ifndef MPI_SETTINGS_H
#define MPI_SETTINGS_H

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
    QVector<qreal> ADC_;
    QVector<MinMax> Sensors_;
    DAC DAC_;
signals:

};

#endif // MPI_SETTINGS_H
