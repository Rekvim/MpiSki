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

<<<<<<< Updated upstream
            emit setDacBlocked(dacRaw, m_task.holdMsecs + m_task.delayMsecs);
            if (m_terminate) break;

            emit StepMeasured(cycle, i, forward);
=======
        setDacBlocked(value, m_task.delayMsecs);
        if (m_terminate) { emit EndTest(); return; }

        setDacBlocked(value, m_task.holdMsecs);
        if (m_terminate) { emit EndTest(); return; }

        emit StepMeasured(rangePercent, 0.0, cycle);

        if ((step + 1) % seqSize == 0) {
            ++cycle;
            emit CycleCompleted(cycle);
>>>>>>> Stashed changes
        }

        if (!m_terminate)
            emit CycleCompleted(cycle + 1);
    }

<<<<<<< Updated upstream
=======
    setDacBlocked(0, 0, true, false);
    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    emit GetPoints(pts);

    TestResults r;

    r.strSequence = seqToString(m_task.sequence);
    r.ranges = calculateRanges(pts, m_task.sequence);
    emit Results(r);

>>>>>>> Stashed changes
    emit EndTest();
}

void CyclicTestsRegulatory::SetTask(Task task)
{
    m_task = task;
}
