#include "CyclicTestPositioner.h"

CyclicTestPositioner::CyclicTestPositioner(QObject *parent)
    : MainTest(parent, false)
{}

void CyclicTestPositioner::Process()
{
    m_cyclicGraphTimer = new QTimer(this);
    connect(m_cyclicGraphTimer, &QTimer::timeout, this, [&] { emit UpdateCyclicTred(); });

    emit SetStartTime();
    m_cyclicGraphTimer->start(500);
    MainTest::Process();

    for (int i = 0; i < m_parameters.num_cycles; ++i) {
        if (m_terminate) {
            emit EndTest();
            return;
        }
        SetDACBlocked(0xFFFF, 0, true, true);
        if (m_terminate) {
            emit EndTest();
            return;
        }
        SetDACBlocked(0, 0, true, true);
    }

    if (m_terminate) {
        emit EndTest();
        return;
    }

    MainTest::Process();
    emit EndTest();
}
