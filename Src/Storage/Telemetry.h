#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include "Src/Domain/Tests/Stroke/StrokeResult.h"
#include "Src/Domain/Tests/Main/MainResult.h"
#include "Src/Domain/Tests/Option/Step/StepResult.h"
#include "Src/Domain/Tests/Cyclic/Regulatory/RegulatoryResult.h"

struct InitState {
    QString deviceStatusText = "";
    QColor deviceStatusColor = QColor();
    QString initStatusText = "";
    QColor initStatusColor = QColor();
    QString connectedSensorsText = "";
    QColor connectedSensorsColor = QColor();
    QString startingPositionText = "";
    QColor startingPositionColor = QColor();
    QString finalPositionText = "";
    QColor finalPositionColor = QColor();
};

struct StepTestRecord {
    quint16 from = 0;
    quint16 to = 0;
    quint32 T_value = 0;
    qreal overshoot = 0.0;
};

struct RangeDeviationRecord {
    qint16 rangePercent = 0;
    double maxForwardValue = 0.0;
    qint16 maxForwardCycle = 0;
    double maxReverseValue = 0.0;
    qint16 maxReverseCycle = 0;
};

struct CyclicTestRecord {
    QString sequenceRegulatory = "";
    QString sequenceShutoff = "";

    quint16 numCyclesRegulatory = 0;
    quint16 numCyclesShutoff = 0;

    double totalTimeSecRegulatory = 0.0;
    double totalTimeSecShutoff = 0.0;

    Domain::Tests::Cyclic::Regulatory::Result regulatoryResult;
    int switch3to0Count = 0;
    int switch0to3Count = 0;

    QVector<quint16> doOnCounts = {};
    QVector<quint16> doOffCounts = {};

    int pos_0to3_hits = 0;
    int pos_0to3_errors = 0;
    int pos_3to0_hits = 0;
    int pos_3to0_errors = 0;
};

struct CrossingStatus {
    enum class State {
        Unknown,
        Ok,
        Fail
    };

    State frictionPercent = State::Unknown;
    State valveStroke = State::Unknown;
    State dynamicError = State::Unknown;
    State spring = State::Unknown;
    State linearCharacteristic = State::Unknown;
};

struct MainTestRecord {
    qreal dynamicError_mean = 0.0;
    qreal dynamicError_meanPercent = 0.0; // %

    qreal dynamicError_max = 0.0;   // %
    qreal dynamicError_maxPercent = 0.0;   // %

    qreal dynamicErrorReal = 0.0;   // %

    qreal lowLimitPressure = 0.0;  // бар
    qreal highLimitPressure = 0.0;  // бар

    qreal springLow = 0.0;  // Н
    qreal springHigh = 0.0;  // Н

    qreal pressureDifference = 0.0;  // бар
    qreal frictionForce = 0.0;  //
    qreal frictionPercent = 0.0;  // %

    qreal linearityError = 0.0;
    qreal linearity = 0.0;
};

struct ValveStrokeRecord {
    QString range = "";
    qreal real= 0.0;
};

struct SupplyRecord {
    double pressure_bar = 0.0;
};

class Telemetry {
public:
    InitState init;
    QVector<StepTestRecord> stepResults;
    CyclicTestRecord cyclicTestRecord;
    ValveStrokeRecord valveStrokeRecord;
    SupplyRecord supplyRecord;
    MainTestRecord mainTestRecord;
    CrossingStatus crossingStatus;

    std::optional<Domain::Tests::Stroke::Result> stroke;
    std::optional<Domain::Tests::Main::Result> main;
    QVector<Domain::Tests::Option::Step::Result> step;
    Domain::Tests::Cyclic::Regulatory::Result regulatory;

    Telemetry() = default;

    void clearAll() {
        init = {};
        stepResults.clear();
        cyclicTestRecord = {};
        valveStrokeRecord = {};
        supplyRecord = {};
        mainTestRecord = {};
    }
};
