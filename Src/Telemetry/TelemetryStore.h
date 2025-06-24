#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once
#include <QVector>

struct StepRecord {
    QString range;
    QString T86sec;
    QString overshoot;
};

struct CyclicTestRecord {
    QString sequence = "";
    quint32 cycles = 0;
    double totalTime = 0.0;

    int switch_3_0_count = 0;
    int switch_0_3_count = 0;
};
struct StrokeTestRecord {
    QString timeForward = "";
    QString timeBackward = "";
};

struct DinamicRecord {
    QString dinamicReal = "";
    QString dinamicRecomend = "";

    QString dinamicIpReal = "";
    QString dinamicIpRecomend = "";
};

struct RangeRecord {
    QString rangeReal = "";
    QString rangeRecomend = "";
    QString rangePressure = "";
};

struct StrokeRecord {
    QString strokeReal = "";
    QString strokeRecomend = "";
};

struct FrictionRecord {
    QString friction = "";
    QString frictionPercent = "";
};

struct SupplyRecord {
    QString supplyPressure = "";
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
