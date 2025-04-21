#include "Sensor.h"

Sensor::Sensor(QObject *parent)
    : QObject{parent}
{
    
}

quint16 Sensor::GetRawValue() const
{
    return value_;
}

quint16 Sensor::GetRawFromValue(qreal raw_value) const
{
    qreal value = (raw_value - b_) / k_;
    if (value < 0)
        return 0;
    if (value > 0xFFFF)
        return 0xFFFF;
    return value;
}

qreal Sensor::GetValue() const
{
    return k_ * value_ + b_;
}

qreal Sensor::GetValueFromPercent(qreal percent)
{
    return k_ * (percent / 100 * (max_value_ - min_value_) + min_value_) + b_;
}

qreal Sensor::GetPersent() const
{
    if (max_value_ == min_value_)
        return 0.0;
    return 100 * qreal(value_ - min_value_) / (max_value_ - min_value_);
}

QString Sensor::GetPersentFormated() const
{
    return QString::asprintf("%.2f %%", GetPersent());
}

QString Sensor::GetFormatedValue() const
{
    return QString::asprintf(unit_.toStdString().c_str(), GetValue());
}

void Sensor::SetValue(quint16 value)
{
    value_ = value;
}

void Sensor::SetCoefficients(qreal k, qreal b)
{
    k_ = k;
    b_ = b;
}

void Sensor::CorrectCoefficients(qreal k)
{
    b_ = -k_ * min_value_;
    if (min_value_ > max_value_) {
        k_ *= -1;
        b_ *= -1;
    }
    k_ *= k;
    b_ *= k;
}

void Sensor::SetMin()
{
    min_value_ = value_;
}

void Sensor::SetMax()
{
    max_value_ = value_;
}

void Sensor::SetUnit(QString unit)
{
    unit_ = "%.2f " + unit;
}
