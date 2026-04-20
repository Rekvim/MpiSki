#pragma once

#include <QMetaType>
#include <QVector>

namespace Domain::Tests::Cyclic::Regulatory {
struct Range
{
    qint16 rangePercent = 0;
    qreal maxForwardPosition = std::numeric_limits<qreal>::lowest();
    int maxForwardCycle = -1;
    qreal minBackwardPosition = std::numeric_limits<qreal>::max();
    int minBackwardCycle = -1;
};
struct Result
{
    QVector<Range> ranges;
};
}
Q_DECLARE_METATYPE(Domain::Tests::Cyclic::Regulatory::Result)
