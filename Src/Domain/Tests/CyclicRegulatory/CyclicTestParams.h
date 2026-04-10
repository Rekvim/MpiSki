#pragma once

#include <QVector>
#include <QMetaType>

struct CyclicTestParams
{
    enum Type {
        Regulatory,
        Shutoff,
        Combined
    }

    testType = Regulatory;

    QVector<qreal> regSeqValues;
    quint16 regulatory_numCycles = 0;
    quint32 regulatory_delayMs = 0;
    quint32 regulatory_holdMs = 0;
    bool regulatory_enable_20mA = false;

    QVector<qreal> offSeqValues;
    quint16 shutoff_numCycles = 0;
    quint32 shutoff_delayMs = 0;
    quint32 shutoff_holdMs = 0;
    std::array<bool,4> shutoff_DO {{false,false,false,false}};
    bool shutoff_DI[2] {false,false};

    void clearRegulatory()
    {
        regSeqValues.clear();
        regulatory_numCycles = 0;
        regulatory_delayMs = 0;
        regulatory_holdMs = 0;
        regulatory_enable_20mA = false;
    }

    void clearShutoff()
    {
        offSeqValues.clear();
        shutoff_numCycles = 0;
        shutoff_delayMs = 0;
        shutoff_holdMs = 0;
        shutoff_DO.fill(false);
    }
};
Q_DECLARE_METATYPE(CyclicTestParams)

