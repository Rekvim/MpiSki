#pragma once

#include <QtGlobal>
#include <QMetaType>

struct StepTestResult {
    quint16 from = 0;
    quint16 to = 0;
    quint32 T_value = 0;
    qreal overshoot = 0.0;
};
Q_DECLARE_METATYPE(StepTestResult)
