#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once

#include <QVector>
#include <QString>
#include <QColor>

struct InitState {
    QString  deviceStatusText = "";
    QColor deviceStatusColor;
    QString initStatusText = "";
    QColor initStatusColor;
    QString connectedSensorsText = "";
    QColor connectedSensorsColor;
    QString startingPositionText = "";
    QColor startingPositionColor = "";
    QString finalPositionText = "";
    QColor finalPositionColor = "";
};

struct SensorState {
    QString linearValue = "";
    QString linearPercent = "";
    QString pressure1 = "";
    QString pressure2 = "";
    QString pressure3 = "";
    QString feedback4_20mA = "";
};

struct StepTestRecord {
    quint16 from;
    quint16 to;
    quint32 T_value;
    qreal overshoot;
};

struct RangeDeviationRecord {
    int rangePercent = 0;
    double  maxForwardValue = 0.0;
    quint32 maxForwardCycle = 0;
    double maxReverseValue = 0.0;
    quint32 maxReverseCycle = 0;
};

struct CyclicTestRecord {
    QString sequence = "";
    quint16 cycles = 0;
    double totalTimeSec = 0;
    QVector<RangeDeviationRecord> ranges;
    int switch3to0Count = 0;
    int switch0to3Count = 0;
};

struct StrokeTestRecord {
    quint64 timeForwardMs = 0;
    quint64 timeBackwardMs = 0;
};

struct MainTestRecord {
    double dynamicError_mean = 0.0;
    double dynamicError_meanPercent = 0.0;

    double dynamicError_max = 0.0;
    double dynamicError_maxPercent = 0.0;

    double dynamicErrorReal = 0.0;

    double lowLimitPressure = 0.0;
    double highLimitPressure = 0.0;

    double springLow = 0.0;
    double springHigh = 0.0;

    double pressureDifference = 0.0;
    double frictionForce = 0.0;
    double frictionPercent = 0.0;
};

struct ValveStrokeRecord {
    QString range = "";
    double real = 0.0;
};

struct SupplyRecord {
    double pressure_bar = 0.0;
};

class TelemetryStore {
public:
    SensorState sensors;
    InitState init;
    QVector<StepTestRecord> stepResults = {{12, 13, 40, 50.02}};
    CyclicTestRecord cyclicTestRecord;
    StrokeTestRecord strokeTestRecord;
    ValveStrokeRecord valveStrokeRecord;
    SupplyRecord supplyRecord;
    MainTestRecord mainTestRecord;

    QVector<int> doOnCounts;
    QVector<int> doOffCounts;

    TelemetryStore() = default;

    void clearAll() {
        // stepResults.clear();
        cyclicTestRecord = {};
        strokeTestRecord = {};
        valveStrokeRecord = {};
        supplyRecord = {};
        doOnCounts.clear();
        doOffCounts.clear();
    }
};

#endif // TELEMETRYSTORE_H
