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
    SetDACBlocked(0, 10000, true);
    if (m_terminate) { emit EndTest(); return; }

    emit SetStartTime();
    m_graphTimer->start(100);

    Sleep(5000);

    if (m_terminate) { emit EndTest(); return; }

    // инициализируем счётчики DO
    int DO_COUNT = m_task.doMask.size();
    m_doOnCounts.assign(DO_COUNT, 0);
    m_doOffCounts.assign(DO_COUNT, 0);
    QVector<bool> currentStates(DO_COUNT, false);

    int perCycle = (m_task.cycles > 0 ? m_task.values.size() / m_task.cycles : m_task.values.size());

    for (int i = 0; i < m_task.values.size() && !m_terminate; ++i) {

        if (perCycle > 0 && (i % perCycle) != 0) {
            for (int d = 0; d < DO_COUNT; ++d) {
                if (!m_task.doMask[d]) continue;
                currentStates[d] = !currentStates[d];
                if (currentStates[d]) ++m_doOnCounts[d];
                else ++m_doOffCounts[d];
            }
            emit SetMultipleDO(currentStates);
        }

        SetDACBlocked(m_task.values.at(i), m_task.delayMs);
        if (m_terminate) { emit EndTest(); return; }

        Sleep(m_task.holdMs);
        if (m_terminate) { emit EndTest(); return; }

        if (perCycle > 0 && (i + 1) % perCycle == 0) {
            emit CycleCompleted((i + 1) / perCycle);
        }
    }

    SetDACBlocked(0, 0, true);
    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    fetchPoints(pts);
    int s3to0 = 0, s0to3 = 0;
    calcSwitchCounts(pts, s3to0, s0to3);

    TestResults r;
    r.doOffCounts = m_doOnCounts;
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
