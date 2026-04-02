#pragma once

#include <QtGlobal>
#include <QMetaType>

struct MainTestResult
{
    qreal pressureDiff = 0.0;
    qreal friction = 0.0;

    qreal dynamicErrorMean = 0.0;
    qreal dynamicErrorMax = 0.0;

    qreal lowLimitPressure = 0.0;
    qreal highLimitPressure = 0.0;

    qreal springLow = 0.0;
    qreal springHigh = 0.0;

    qreal linearityError = 0.0;
    qreal linearity = 0.0;
};
Q_DECLARE_METATYPE(MainTestResult)
