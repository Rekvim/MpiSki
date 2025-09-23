#include "CyclicTestsShutoff.h"
#include <QElapsedTimer>
#include <QThread>

CyclicTestsShutoff::CyclicTestsShutoff(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false)
{}

void CyclicTestsShutoff::SetTask(const Task& task)
{
    m_task = task;
}

void CyclicTestsShutoff::Process()
{
    emit ClearGraph();

    emit SetStartTime();
    m_graphTimer->start(100);

    // инициализируем счётчики DO
    int DO_COUNT = m_task.doMask.size();
    m_doOnCounts.assign(DO_COUNT, 0);
    m_doOffCounts.assign(DO_COUNT, 0);
    QVector<bool> currentStates(DO_COUNT, false);

    int perCycle = (m_task.cycles > 0 ? m_task.values.size() / m_task.cycles : m_task.values.size());

    for (quint32 step = 0; step < m_task.values.size() && !m_terminate; ++step) {
        const quint16 value = m_task.values.at(step);

        if (perCycle > 0 && (step % perCycle) != 0) {
            for (int d = 0; d < DO_COUNT; ++d) {
                if (!m_task.doMask[d]) continue;
                currentStates[d] = !currentStates[d];
                if (currentStates[d]) ++m_doOnCounts[d];
                else ++m_doOffCounts[d];
            }
            emit SetMultipleDO(currentStates);
        }

        SetDACBlocked(value, m_task.delayMsecs);
        if (m_terminate) { emit EndTest(); return; }

        SetDACBlocked(value, m_task.holdMsecs);
        if (m_terminate) { emit EndTest(); return; }

        if (perCycle > 0 && (step + 1) % perCycle == 0) {
            emit CycleCompleted((step + 1) / perCycle);
        }
    }

    SetDACBlocked(0, 0, true);
    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    fetchPoints(pts);
    int s3to0 = 0, s0to3 = 0;
    calcSwitchCounts(pts, s3to0, s0to3);

    TestResults r;
    r.doOnCounts = m_doOnCounts;
    r.doOffCounts = m_doOffCounts;
    r.switch3to0Count = s3to0;
    r.switch0to3Count = s0to3;

    emit Results(r);
    emit EndTest();
}

void CyclicTestsShutoff::fetchPoints(QVector<QVector<QPointF>>& pts)
{
    pts.clear();
    emit GetPoints(pts);
}

void CyclicTestsShutoff::calcSwitchCounts(const QVector<QVector<QPointF>>& pts,
                                          int& s3to0, int& s0to3) const
{
    s3to0 = s0to3 = 0;
    if (pts.size() < 2) return;

    const auto& opened = pts.at(0); // переход 0→3
    const auto& closed = pts.at(1); // переход 3→0
    s0to3 = opened.size();
    s3to0 = closed.size();
}
