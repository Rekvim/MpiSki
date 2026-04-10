#pragma once

#include <QMetaType>
#include <QVector>

struct CyclicRangeResult
{
    qint16 rangePercent = 0;

    qreal maxForwardPosition = std::numeric_limits<qreal>::lowest();
    int maxForwardCycle = -1;

    qreal minReversePosition = std::numeric_limits<qreal>::max();
    int minReverseCycle = -1;
};

struct CyclicRegulatoryResult
{
    QVector<CyclicRangeResult> ranges;
};

Q_DECLARE_METATYPE(CyclicRegulatoryResult)
