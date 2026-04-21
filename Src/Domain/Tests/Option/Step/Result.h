#pragma once

#include <QMetaType>

namespace Domain::Tests::Option::Step {
    struct Result {
        quint16 from = 0;
        quint16 to = 0;
        quint32 T_value = 0;
        qreal overshoot = 0.0;
    };
}
Q_DECLARE_METATYPE(Domain::Tests::Option::Step::Result)
