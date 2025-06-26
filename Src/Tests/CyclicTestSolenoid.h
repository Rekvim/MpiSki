// CyclicTestSolenoid.h  — без изменений
#ifndef CYCLICTESTSOLENOID_H
#define CYCLICTESTSOLENOID_H

#pragma once

#include "MainTest.h"
#include "CyclicTestSettings.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Src/MPI/MPI.h"

#include <QElapsedTimer>
#include <QThread>
#include <QVector>
#include <QList>
#include <QString>

class CyclicTestSolenoid : public MainTest {
    Q_OBJECT

public:
    using Parameters = CyclicTestSettings::TestParameters;

    explicit CyclicTestSolenoid(QObject* parent = nullptr);

    void SetParameters(const Parameters& params);

public slots:
    void Process() override;

signals:
    void SetMultipleDO(const QVector<bool>& states);
    void SetDO(quint8 index, bool state);
    void TaskPoint(quint64 timeMs, int percent);
    void UpdateCyclicTred();
    void SetStartTime();

    void SetSolenoidResults(QString sequence,
                            quint16 cycles,
                            double totalTimeSec);
    void CyclicDeviationResults(const QVector<RangeDeviationRecord>& recs);
    void DOCounts(const QVector<int>& onCounts, const QVector<int>& offCounts);
    void RegulatoryMeasurement(int cycle, bool forward);

private:
    MPI   m_mpi;
    Parameters   m_params;
    QVector<int> m_valuesReg;
    QVector<int> m_valuesOff;

    void parseSequence(const QString& seq, QVector<int>& out);
    void runLoop(const QVector<int>& values,
                 int delayMs,
                 int holdMs,
                 int cycles);

    void processRegulatory();
    void processShutoff();
    void processCombined();
};

#endif // CYCLICTESTSOLENOID_H
