// CyclicTestSolenoid.h
#ifndef CYCLICTESTSOLENOID_H
#define CYCLICTESTSOLENOID_H

#pragma once

#include "MainTest.h"
#include <QThread>
#include <QDateTime>

class CyclicTestSolenoid : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestSolenoid(QObject *parent = nullptr);

    void SetParameters(const QString &sequence,
                       int delaySec,
                       int holdTimeSec,
                       int numCycles);

public slots:

    void Process() override;

signals:
    void ClearGraph();
    void TaskPoint(quint64 timeMs, int percent);
    void UpdateCyclicTred();
    void SetStartTime();

    void SetSolenoidResults(QString sequence, double forwardSec, double backwardSec, quint16 cycles, double rangePercent,  double totalTimeSec);

private:
    QString m_sequence;
    QList<int> m_dacValues;
    int m_stepDelayMs = 0;
    int m_holdTimeMs  = 0;
    int m_numCycles   = 0;
    quint64 m_startTime = 0;
};

#endif // CYCLICTESTSOLENOID_H
