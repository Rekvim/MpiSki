#include "MySeries.h"

MySeries::MySeries(QObject *parent, quint8 axN)
    : QLineSeries{parent}
{
    m_axisN = axN;
}

quint8 MySeries::getAxisN() const
{
    return m_axisN;
}
