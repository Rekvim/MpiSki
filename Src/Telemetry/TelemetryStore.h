#ifndef TELEMETRYSTORE_H
#define TELEMETRYSTORE_H

#pragma once
#include <QVector>

struct StepRecord {
    QString range;
    double T86sec;
    double overshoot;
};

struct CyclicRecord {
    int cycle;
    double timeSec;

};


class TelemetryStore {
public:
    QVector<StepRecord> stepResults;
    QVector<CyclicRecord> cyclicResults;

    void clearAll() {
        stepResults.clear();
        cyclicResults.clear();
     }
};

#endif // TELEMETRYSTORE_H
