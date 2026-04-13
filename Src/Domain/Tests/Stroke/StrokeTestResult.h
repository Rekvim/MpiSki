#pragma once

#include <QMetaType>

struct StrokeTestResult {
    QString timeForwardMs = "";
    QString timeBackwardMs = "";

    quint64 forwardTimeMs = 0;
    quint64 backwardTimeMs = 0;
};
Q_DECLARE_METATYPE(StrokeTestResult)
