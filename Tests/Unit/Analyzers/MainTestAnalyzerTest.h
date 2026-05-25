#pragma once

#include <QObject>

class MainTestAnalyzerTest : public QObject
{
    Q_OBJECT

private slots:
    // ── Существующие тесты ────────────────────────────────────────────────
    void emptyInput_resultIsAllZero();
    void nanSamples_ignoredAndDoNotCrash();
    void onlyForwardStroke_resultIsZero();
    void perfectLinear_noHysteresis_pressureDiffIsZero();
    void perfectLinear_linearityErrorIsZero();
    void knownHysteresis_pressureDiffMatchesExpected();
    void knownHysteresis_frictionPercentMatchesExpected();
    void start_resetsResult();

    // ── Новые тесты ───────────────────────────────────────────────────────
    void dynamicError_uniformDacOffset_computesCorrectly();
    void frictionForce_knownDiameter_matchesFormula();
    void springAndPressureLimits_parallelRegressions_computeCorrectly();
    void allPointsOutsideRange_regressionFails_resultIsZero();
    void minimumPoints_exactlyTwoInRange_regressionSucceeds();
};
