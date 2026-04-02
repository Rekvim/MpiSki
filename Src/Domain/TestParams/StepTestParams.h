#pragma once

#include <QVector>
#include <QMetaType>

struct StepTestParams
{
    quint32 delay;
    qreal testValue;
    QVector<qreal> points;
};
Q_DECLARE_METATYPE(StepTestParams)
