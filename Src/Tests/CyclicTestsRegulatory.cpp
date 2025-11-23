#include "CyclicTestsRegulatory.h"

CyclicTestsRegulatory::CyclicTestsRegulatory(QObject* parent, bool endTestAfterProcess)
    : MainTest(parent),
    m_endTestAfterProcess(endTestAfterProcess)
{}

void CyclicTestsRegulatory::Process()
{
    emit SetStartTime();
    emit ClearGraph();

    m_graphTimer->start(250);

    const auto& raw = m_task.values;
    const auto& seq = m_task.sequence;

    for (quint32 cycle = 0; cycle < m_task.cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < raw.size() && !m_terminate; ++i) {
            const quint16 dacRaw = raw.at(i);

            const int pct = seq.at(i);
            const int prevPct = (i == 0 ? seq.last() : seq.at(i - 1));
            const bool forward = (pct > prevPct);

            emit setDacBlocked(dacRaw, m_task.holdMsecs + m_task.delayMsecs);
            if (m_terminate) break;

            emit StepMeasured(cycle, i, forward);
        }

        if (!m_terminate)
            emit CycleCompleted(cycle + 1);
    }

    emit EndTest();
}

void CyclicTestsRegulatory::SetTask(Task task)
{
    m_task = task;
}
