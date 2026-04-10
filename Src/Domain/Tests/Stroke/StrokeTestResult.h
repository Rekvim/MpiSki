#pragma once

#include <QString>

struct StrokeTestResult {
    QString timeForwardMs = "";
    QString timeBackwardMs = "";

    quint64 forwardTimeMs = 0;
    quint64 backwardTimeMs = 0;
};
