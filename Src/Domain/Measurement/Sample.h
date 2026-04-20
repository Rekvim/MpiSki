#pragma once

#include <QtGlobal>
#include <QMetaType>
#include <QtMath>

namespace Domain::Measurement {
    struct Sample {
        qint64 systemTime = 0;
        qint64 testTime = 0;

        qreal dac = 0.0;

        qreal taskPercent = 0.0;

        qreal positionValue = 0.0;
        qreal positionPercent = 0.0;

        quint8 diMask = 0;
        quint8 doMask = 0;

        qreal pressure1 = qQNaN();
        qreal pressure2 = qQNaN();
        qreal pressure3 = qQNaN();

        qreal feedbackCurrent = qQNaN();

        QString positionUnit;
    };
}
Q_DECLARE_METATYPE(Domain::Measurement::Sample)
