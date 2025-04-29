#include "Sensor.h"

Sensor::Sensor(QObject *parent)
    : QObject{parent}
{
    
}

quint16 Sensor::GetRawValue() const
{
    return m_value;
}

quint16 Sensor::GetRawFromValue(qreal raw_value) const
{
    qreal value = (raw_value - m_b) / m_k;
    if (value < 0)
        return 0;
    if (value > 0xFFFF)
        return 0xFFFF;
    return value;
}

qreal Sensor::GetValue() const
{
    return m_k * m_value + m_b;
}

qreal Sensor::GetValueFromPercent(qreal percent)
{
    return m_k * (percent / 100 * (m_maxValue - m_minValue) + m_minValue) + m_b;
}

qreal Sensor::GetPersent() const
{
    if (m_maxValue == m_minValue)
        return 0.0;
    return 100 * qreal(m_value - m_minValue) / (m_maxValue - m_minValue);
}

QString Sensor::GetPersentFormated() const
{
    return QString::asprintf("%.2f %%", GetPersent());
}

QString Sensor::GetFormatedValue() const
{
    return QString::asprintf(m_unit.toStdString().c_str(), GetValue());
}

void Sensor::SetValue(quint16 value)
{
    m_value = value;
}

void Sensor::SetCoefficients(qreal k, qreal b)
{
    m_k = k;
    m_b = b;
}

void Sensor::CorrectCoefficients(qreal k)
{
    m_b = -m_k * m_minValue;
    if (m_minValue > m_maxValue) {
        m_k *= -1;
        m_b *= -1;
    }
    m_k *= k;
    m_b *= k;
}

void Sensor::SetMin()
{
    m_minValue = m_value;
}

void Sensor::SetMax()
{
    m_maxValue = m_value;
}

void Sensor::SetUnit(QString unit)
{
    m_unit = "%.2f " + unit;
}
