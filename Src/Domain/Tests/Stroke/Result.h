#pragma once

#include <QMetaType>

namespace Domain::Tests::Stroke {
    struct Result {
        QString timeForwardMs = "";
        QString timeBackwardMs = "";

        quint64 forwardTimeMs = 0;
        quint64 backwardTimeMs = 0;
    };
}
Q_DECLARE_METATYPE(Domain::Tests::Stroke::Result)
