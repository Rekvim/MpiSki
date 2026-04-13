#pragma once

#include <QVector>
#include <QMetaType>
#include "Regulatory/CyclicRegulatoryParams.h"
#include "Shutoff/CyclicShutoffParams.h"

struct CyclicTestParams
{
    enum Type
    {
        Regulatory,
        Shutoff,
        Combined
    };

    Type type = Regulatory;

    CyclicRegulatoryParams regulatory;
    CyclicShutoffParams shutoff;
};

Q_DECLARE_METATYPE(CyclicTestParams)

