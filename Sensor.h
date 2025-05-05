#ifndef SENSOR_H
#define SENSOR_H

#pragma once
#include <QObject>

class Sensor : public QObject
{
    Q_OBJECT
public:
    explicit Sensor(QObject *parent = nullptr);
    quint16 GetRawValue() const;
    quint16 GetRawFromValue(qreal raw_value) const;
    qreal GetValue() const;
    qreal GetValueFromPercent(qreal percent);
    qreal GetPersent() const;
    QString GetPersentFormated() const;
    QString GetFormatedValue() const;
    void SetValue(quint16 value);
    void SetCoefficients(qreal k, qreal b);
    void CorrectCoefficients(qreal k);
    void SetMin();
    void SetMax();
    void SetUnit(QString unit);

private:
    quint16 m_value;
    quint16 m_minValue;
    quint16 m_maxValue;
    QString m_unit = "%.2f";
    qreal m_k = 1;
    qreal m_b = 0;

};

#endif // SENSOR_H
