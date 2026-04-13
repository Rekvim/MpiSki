#pragma once

#include <QMetaType>
#include <QVector>

struct CyclicShutoffResult
{
    quint16 numCycles = 0;

    QVector<quint16> doOnCounts;
    QVector<quint16> doOffCounts;

    quint16 switch3to0Count = 0;
    quint16 switch0to3Count = 0;
};
Q_DECLARE_METATYPE(CyclicShutoffResult)
