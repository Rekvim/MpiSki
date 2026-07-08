#pragma once

#include <QMetaType>

namespace Domain::Tests::Main {

    struct RegressionContext {
        double k1 = 0, b1 = 0;  // прямой ход: position = k1 * pressure + b1
        double k2 = 0, b2 = 0;  // обратный ход
        double limMinX = 0, limMaxX = 0;
        double limMinY = 0, limMaxY = 0;
        bool valid = false;
    };

    struct Result {
        qreal pressureDiff = 0.0; // бар

        qreal frictionForce = 0.0; // Н
        qreal frictionPercent = 0.0; // %

        qreal dynamicErrorMean = 0.0; // мА
        qreal dynamicErrorMeanPercent = 0.0; // %

        qreal dynamicErrorMax = 0.0; // мА
        qreal dynamicErrorMaxPercent = 0.0; // %

        qreal dynamicErrorReal = 0.0; // %

        qreal lowLimitPressure = 0.0; // бар
        qreal highLimitPressure = 0.0; // бар

        qreal springLow = 0.0; // Н
        qreal springHigh = 0.0; // Н

        qreal linearityError = 0.0; // %
        qreal linearity = 0.0; // %

        RegressionContext regressionCtx;
    };
}
Q_DECLARE_METATYPE(Domain::Tests::Main::Result)