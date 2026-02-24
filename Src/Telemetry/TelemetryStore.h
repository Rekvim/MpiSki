#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include <QMetaType>

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
    QString sequenceRegulatory = "0-25-50-75-100";
    QString sequenceShutoff = "0-100";

    quint16 numCyclesRegulatory = 5;
    quint16 numCyclesShutoff = 10;

    double totalTimeSecRegulatory = 150.0;
    double totalTimeSecShutoff = 180.0;

    QVector<RangeDeviationRecord> ranges = {
        { 0,   1.8,  2,  -1.2, 4 },
        { 25, 26.5,  1,  23.7, 3 },
        { 50, 51.3,  3,  48.9, 2 },
        { 75, 76.2,  4,  73.5, 1 },
        { 100,101.1, 2,  98.6, 4 }
    };
    int switch3to0Count = 8;
    int switch0to3Count = 8;

    QVector<quint16> doOnCounts = {8, 8, 0, 0};
    QVector<quint16> doOffCounts = {8, 8, 0, 0};

    int pos_0to3_hits = 8;
    int pos_0to3_errors = 0;
    int pos_3to0_hits = 8;
    int pos_3to0_errors = 0;
};

struct StrokeTestRecord {
    QString timeForwardMs = "";
    QString timeBackwardMs = "";
};

struct CrossingStatus {
    enum class State {
        Unknown,
        Ok,
        Fail
    };

    State frictionPercent = State::Unknown;
    State range = State::Unknown;
    State dynamicError = State::Unknown;
    State spring = State::Unknown;
    State linearCharacteristic = State::Unknown;
};


struct MainTestRecord {
    double dynamicError_mean = 0.0;
    double dynamicError_meanPercent = 0.0; // %

    double dynamicError_max = 0.0;   // %
    double dynamicError_maxPercent = 0.0;   // %

    double dynamicErrorReal = 0.0;   // %

    double lowLimitPressure = 0.0;  // бар
    double highLimitPressure = 0.0;  // бар

    double springLow = 0.0;  // Н
    double springHigh = 0.0;  // Н

    double pressureDifference = 0.0;  // бар
    double frictionForce = 0.0;  //
    double frictionPercent = 0.0;  // %

    double linearityError = 0.0;
    double linearity = 0.0;
};

struct ValveStrokeRecord {
    QString range = "";
    qreal real= 0.0;
};

struct SupplyRecord {
    double pressure_bar = 0.0;
};

class TelemetryStore {
public:
    InitState init;
    QVector<StepTestRecord> stepResults;
    CyclicTestRecord cyclicTestRecord;
    StrokeTestRecord strokeTestRecord;
    ValveStrokeRecord valveStrokeRecord;
    SupplyRecord supplyRecord;
    MainTestRecord mainTestRecord;
    CrossingStatus crossingStatus;

    TelemetryStore() = default;

    void clearAll() {
        init = {};
        stepResults.clear();
        cyclicTestRecord = {};
        strokeTestRecord = {};
        valveStrokeRecord = {};
        supplyRecord = {};
        mainTestRecord = {};
    }
};
