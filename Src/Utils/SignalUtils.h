#pragma once

#include <QVector>

#include "Domain/Mpi/Device.h"
#include "Domain/Measurement/Sensor.h"

namespace SignalUtils
{

inline qreal
calcPercent(qreal value, bool invert = false)
{
    qreal percent = ((value - 4.0) / 16.0) * 100.0;
    percent = qBound<qreal>(0.0, percent, 100.0);
    return invert ? (100.0 - percent) : percent;
}
inline QVector<quint16>
makeRawValues(const QVector<qreal>& seq, Domain::Mpi::Device& device, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.size());

    for (quint16 pct : seq) {
        const qreal current = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        raw.push_back(device.dac()->rawFromValue(current));
    }
    return raw;
}
}
