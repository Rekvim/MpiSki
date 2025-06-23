#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once
#include <QVector>

struct StepRecord {
    QString range;
    double T86sec;
    qreal overshoot;
};

struct CyclicTestRecord {
    QString sequence = "";
    quint32 cycles = 0;
    double totalTime = 0.0;
};
struct StrokeTestRecord {
    double timeForward = 0.0;
    double timeBackward = 0.0;
};

struct DinamicRecord {
    double dinamicReal = 0.0;
    double dinamicRecomend = 0.0;

    double dinamicIpReal = 0.0;
    double dinamicIpRecomend = 0.0;
};

struct RangeRecord {
    double rangeReal = 0.0;
    double rangeRecomend = 0.0;
    double rangePressure = 0.0;
};

struct StrokeRecord {
    double strokeReal = 0.0;
    double strokeRecomend = 0.0;
};

struct FrictionRecord {
    double friction = 0.0;
    double frictionPercent = 0.0;
};

struct SupplyRecord {
    double supplyPressure = 0.0;
};

class TelemetryStore {
public:
    QVector<StepRecord> stepResults;

    CyclicTestRecord cyclicTestRecord;
    StrokeTestRecord strokeTestRecord;
    DinamicRecord dinamicRecord;
    RangeRecord rangeRecord;
    StrokeRecord strokeRecord;
    FrictionRecord frictionRecord;
    SupplyRecord supplyRecord;

    void clearAll() {
        stepResults.clear();
        cyclicTestRecord = {};
        strokeTestRecord = {};
        dinamicRecord = {};
        rangeRecord = {};
        strokeRecord = {};
        frictionRecord = {};
        supplyRecord = {};
    }
};

#endif // TELEMETRYSTORE_H
