#include "Sensor.h"

Sensor::Sensor(QObject *parent)
    : QObject(parent)
{}

// --- getters ---

quint16 Sensor::rawValue() const
{
    return m_value;
}

quint16 Sensor::rawFromValue(qreal physicalValue) const
{
    if (m_k == 0.0)
        return 0;

    qreal raw = (physicalValue - m_b) / m_k;

    if (raw < 0.0)
        raw = 0.0;
    else if (raw > 65535.0)
        raw = 65535.0;

    return static_cast<quint16>(qRound(raw));
}

qreal Sensor::value() const
{
    return m_k * m_value + m_b;
}

qreal Sensor::valueFromPercent(qreal percent) const
{
    return m_k * (percent / 100.0 * (m_maxValue - m_minValue) + m_minValue) + m_b;
}

qreal Sensor::percent() const
{
    if (m_maxValue == m_minValue)
        return 0.0;

    return 100.0 * qreal(m_value - m_minValue) / (m_maxValue - m_minValue);
}

QString Sensor::percentFormatted() const
{
    return QString::asprintf("%.2f %%", percent());
}

QString Sensor::formattedValue() const
{
    return QString::asprintf(m_unit.toStdString().c_str(), value());
}

// --- mutators ---

void Sensor::setValue(quint16 value)
{
    m_value = value;
}

void Sensor::setCoefficients(qreal k, qreal b)
{
    m_k = k;
    m_b = b;
}

void Sensor::correctCoefficients(qreal scale)
{
    // Нормируем так, чтобы при minValue → 0 в инженерных единицах
    m_b = -m_k * m_minValue;

    if (m_minValue > m_maxValue) {
        m_k *= -1;
        m_b *= -1;
    }

    m_k *= scale;
    m_b *= scale;
}

void Sensor::captureMin()
{
    m_minValue = m_value;
}

void Sensor::captureMax()
{
    m_maxValue = m_value;
}

void Sensor::setUnit(const QString &unit)
{
    m_unit = "%.2f " + unit;
}
