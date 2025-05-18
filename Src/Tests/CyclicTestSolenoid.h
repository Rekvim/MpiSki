#ifndef CYCLICTESTSOLENOID_H
#define CYCLICTESTSOLENOID_H

#pragma once
#include "MainTest.h"
#include <QTimer>
#include <QStringList>

class CyclicTestSolenoid : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestSolenoid(QObject *parent = nullptr);
    void SetParameters(const QString &sequence, int delay_sec, int num_cycles);

public slots:
    void Process() override;

signals:
    void SetStartTime();
    void UpdateCyclicTred();

private:
    QList<int> m_dacValues;
    int m_holdTimeMs;
    int m_numCycles;
    QTimer *m_cyclicGraphTimer = nullptr;
};

#endif // CYCLICTESTSOLENOID_H
