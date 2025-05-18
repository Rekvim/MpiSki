#include "CyclicTestSolenoid.h"
#include <QTimer>

CyclicTestSolenoid::CyclicTestSolenoid(QObject *parent)
    : MainTest(parent, false) {}

void CyclicTestSolenoid::SetParameters(const QString &sequence, int delay_sec, int num_cycles) {
    m_dacValues.clear();
    for (const QString &val : sequence.split("-", Qt::SkipEmptyParts))
        m_dacValues.append(val.toInt());

    m_holdTimeMs = delay_sec * 1000;
    m_numCycles = num_cycles;
}

void CyclicTestSolenoid::Process() {
    emit SetStartTime();
    emit ClearGraph();

    m_cyclicGraphTimer = new QTimer(this);
    connect(m_cyclicGraphTimer, &QTimer::timeout, this, [&] { emit UpdateCyclicTred(); });
    m_cyclicGraphTimer->start(500);

    for (int cycle = 0; cycle < m_numCycles && !m_terminate; ++cycle) {
        for (int val : m_dacValues) {
            if (m_terminate) break;
            SetDAC(val, m_holdTimeMs, true, true);
        }
    }

    emit EndTest();
}
