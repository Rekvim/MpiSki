#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include <QMetaType>

struct InitState {
    QString  deviceStatusText = "";
    QColor deviceStatusColor  = "";
    QString initStatusText = "";
    QColor initStatusColor  = "";
    QString connectedSensorsText = "";
    QColor connectedSensorsColor;
    QString startingPositionText = "";
    QColor startingPositionColor = "";
    QString finalPositionText = "";
    QColor finalPositionColor = "";
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
    QString sequence = "";
    quint16 cycles = 0;
    double totalTimeSec = 0;
    QVector<RangeDeviationRecord> ranges;
    int switch3to0Count = 0;
    int switch0to3Count = 0;

    QVector<quint16> doOnCounts;
    QVector<quint16> doOffCounts;

    int pos_0to3_hits = 0;
    int pos_0to3_errors = 0;
    int pos_3to0_hits = 0;
    int pos_3to0_errors = 0;
};

struct StrokeTestRecord {
    QString timeForwardMs = "";
    QString timeBackwardMs = "";
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
    InitState init;
    QVector<StepTestRecord> stepResults;
    CyclicTestRecord cyclicTestRecord;
    StrokeTestRecord strokeTestRecord;
    ValveStrokeRecord valveStrokeRecord;
    SupplyRecord supplyRecord;
    MainTestRecord mainTestRecord;

    TelemetryStore() = default;

    void clearAll() {
        init = {};
        strokeTestRecord = {};
        mainTestRecord = {};
        //stepResults.clear();
        cyclicTestRecord = {};
        valveStrokeRecord = {};
        supplyRecord = {};
    }
};

#endif // TELEMETRYSTORE_H
