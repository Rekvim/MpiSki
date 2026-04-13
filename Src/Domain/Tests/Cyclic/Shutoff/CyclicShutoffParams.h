#pragma once

#include <QVector>
#include <QMetaType>

struct CyclicShutoffParams
{
    QVector<qreal> sequence;

    quint16 numCycles = 0;

    quint32 delayMs = 0;
    quint32 holdMs = 0;

    std::array<bool,4> DO {{false,false,false,false}};
    std::array<bool,2> DI {{false,false}};
};

Q_DECLARE_METATYPE(CyclicShutoffParams)
