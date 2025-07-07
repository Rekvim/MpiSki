#ifndef MPISETTINGS_H
#define MPISETTINGS_H

#pragma once
#include <QObject>
#include <QApplication>
#include <QSettings>

class MpiSettings : public QObject
{
    Q_OBJECT
public:
    explicit MpiSettings(QObject *parent = nullptr);

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
    QVector<MinMax> m_sensors;
    DAC m_DAC;
signals:

};

#endif // MPISETTINGS_H
