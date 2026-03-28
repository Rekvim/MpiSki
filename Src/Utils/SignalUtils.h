#pragma once

#include <QString>
#include <QLineEdit>

namespace SignalUtils
{

inline qreal
calcPercent(qreal value, bool invert = false)
{
    qreal percent = ((value - 4.0) / 16.0) * 100.0;
    percent = qBound<qreal>(0.0, percent, 100.0);
    return invert ? (100.0 - percent) : percent;
}

}
