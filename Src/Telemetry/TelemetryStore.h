#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include <QMetaType>

struct InitState {
    QString deviceStatusText      = "Устройство готово";
    QColor  deviceStatusColor     = QColor("green");
    QString initStatusText        = "Инициализация завершена";
    QColor  initStatusColor       = QColor("#2E7D32");
    QString connectedSensorsText  = "Датчики: P1, P2, P3 подключены";
    QColor  connectedSensorsColor = QColor("blue");
    QString startingPositionText  = "Стартовая позиция: 0%";
    QColor  startingPositionColor = QColor("darkgray");
    QString finalPositionText     = "Финальная позиция: 100%";
    QColor  finalPositionColor    = QColor("darkgray");
};

struct StepTestRecord {
    quint16 from      = 0;
    quint16 to        = 25;
    quint32 T_value   = 180;
    qreal   overshoot = 1.2;
};

struct RangeDeviationRecord {
    qint16 rangePercent     = 0;
    double maxForwardValue  = 1.1;
    qint16 maxForwardCycle  = 23;
    double maxReverseValue  = 1.2;
    qint16 maxReverseCycle  = 22;
};

struct CyclicTestRecord {
    QString sequence                 = "0-1-2-3-2-1-0";
    quint16 cycles                   = 120;
    double  totalTimeSec             = 3625.4;
    QVector<RangeDeviationRecord> ranges = {
        RangeDeviationRecord{-100, 0.6, 12, 0.7, 11},
        RangeDeviationRecord{ -50, 0.9, 18, 1.0, 17},
        RangeDeviationRecord{   0, 1.1, 23, 1.2, 22},
        RangeDeviationRecord{  50, 1.4, 27, 1.5, 28},
        RangeDeviationRecord{ 100, 1.9, 31, 2.0, 30},
    };
    int switch3to0Count = 58;
    int switch0to3Count = 62;

    QVector<quint16> doOnCounts  = {120, 119, 118, 120};
    QVector<quint16> doOffCounts = {121, 120, 119, 121};

    int pos_0to3_hits   = 118;
    int pos_0to3_errors = 2;
    int pos_3to0_hits   = 117;
    int pos_3to0_errors = 3;
};

struct StrokeTestRecord {
    QString timeForwardMs  = "125";
    QString timeBackwardMs = "131";
};

struct MainTestRecord {
    double dynamicError_mean        = 0.48;
    double dynamicError_meanPercent = 0.48;

    double dynamicError_max         = 1.95;
    double dynamicError_maxPercent  = 1.95;

    double dynamicErrorReal         = 0.62;

    double lowLimitPressure         = 1.1;  // бар
    double highLimitPressure        = 4.9;  // бар

    double springLow                = 120.0; // Н
    double springHigh               = 245.0; // Н

    double pressureDifference       = 3.85;  // бар
    double frictionForce            = 12.6;  // Н
    double frictionPercent          = 3.2;   // %
};

struct ValveStrokeRecord {
    QString range = "0–100%";
    double  real  = 98.7;
};

struct SupplyRecord {
    double pressure_bar = 5.2;
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
