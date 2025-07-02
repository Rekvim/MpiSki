#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once

#include <QVector>
#include <QString>

/** Результат одного шага StepTest */
struct StepRecord {
    QString range;
    qint64 T_ms; // время достижения T-значения в миллисекундах (0 = ошибка)
    double overshootPct; // перерегулирование, %
};

/** Описание точки в циклическом тесте */
struct RangeDeviationRecord {
    int rangePercent = 0;
    double  maxForwardValue = 0.0;
    quint32 maxForwardCycle = 0;
    double maxReverseValue = 0.0;
    quint32 maxReverseCycle = 0;
};

/** Полные результаты циклического теста */
struct CyclicTestRecord {
    QString sequence; // например {0,50,100}
    quint16 cycles = 0;
    double totalTimeSec = 0; // миллисекунды
    QVector<RangeDeviationRecord> ranges;
    int switch3to0Count = 0;
    int switch0to3Count = 0;
};

/** StrokeTest */
struct StrokeTestRecord {
    quint64 timeForwardMs = 0;
    quint64 timeBackwardMs = 0;
};

struct MainTestRecord {
    double dynamicError_mean = 0.0;
    double dynamicError_meanPercent = 0.0;

    double dynamicError_max = 0.0;
    double dynamicError_maxPercent = 0.0;

    double dynamicReal = 0.0;

    double lowLimit = 0.0;
    double highLimit = 0.0;

    double springLow = 0.0;
    double springHigh = 0.0;

    double pressureDifference = 0.0;
    double frictionForce = 0.0; // сила трения
    double frictionPercent = 0.0;  // в процентах
};

/** Stroke (ход штока) */
struct StrokeRecord {
    double strokeReal = 0.0;
    double strokeRecomend = 0.0;
};

/** Supply Pressure */
struct SupplyRecord {
    double pressure_bar = 0.0;
};

/** Хранилище всех телеметрийных данных */
class TelemetryStore {
public:
    QVector<StepRecord> stepResults;
    CyclicTestRecord cyclicTestRecord;
    StrokeTestRecord strokeTestRecord;
    StrokeRecord strokeRecord;
    SupplyRecord supplyRecord;
    MainTestRecord mainTestRecord;

    QVector<int> doOnCounts;
    QVector<int> doOffCounts;

    void clearAll() {
        stepResults.clear();
        cyclicTestRecord = {};
        strokeTestRecord = {};
        strokeRecord     = {};
        supplyRecord     = {};
        doOnCounts.clear();
        doOffCounts.clear();
    }
};

#endif // TELEMETRYSTORE_H
