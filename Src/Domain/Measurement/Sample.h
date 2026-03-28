#pragma once

#include <QtGlobal>
#include <QMetaType>
#include <QtMath>

struct Sample
{
    qint64 time = 0;

    qreal dac = 0.0;

    qreal taskPercent = 0.0;
    qreal positionPercent = 0.0;

    qreal pressure1 = qQNaN();
    qreal pressure2 = qQNaN();
    qreal pressure3 = qQNaN();
};

Q_DECLARE_METATYPE(Sample)