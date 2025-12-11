#include "CyclicShutoffRunner.h"
#include "./Program.h"

static QVector<quint16> makeRawValues(const QVector<qreal>& seq, Mpi& mpi, bool normalOpen) {
    QVector<quint16> raw; raw.reserve(seq.size());
    for (quint16 pct : seq) {
        const qreal cur = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        raw.push_back(mpi.GetDac()->rawFromValue(cur));
    }
    return raw;
}

RunnerConfig CyclicShutoffRunner::buildConfig() {
    const auto& p = m_params;

    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    if (p.testType != p.Type::Shutoff) return {};

    const bool normalOpen = (m_reg.getValveInfo()->safePosition != 0);
    const auto raw = makeRawValues(p.offSeqValues, m_mpi, normalOpen);

    CyclicTestsShutoff::Task task;
    task.delayMsecs = p.shutoff_delayMs;
    task.holdMsecs  = p.shutoff_holdMs;
    task.cycles = p.shutoff_numCycles;
    task.doMask = QVector<bool>(p.shutoff_DO.begin(), p.shutoff_DO.end());

    task.values.reserve(p.shutoff_numCycles * raw.size());
    for (int c = 0; c < p.shutoff_numCycles; ++c) task.values += raw;

    auto* worker = new CyclicTestsShutoff;
    worker->SetTask(task);

    const quint64 steps = static_cast<quint64>(raw.size()) * p.shutoff_numCycles;
    const quint64 totalMs = steps * (p.shutoff_delayMs + p.shutoff_holdMs);

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Cyclic);
    return cfg;
}

void CyclicShutoffRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<CyclicTestsShutoff&>(base);
    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    connect(&t, &CyclicTestsShutoff::UpdateGraph,
            owner, [owner]{ owner->updateCharts_CyclicTest(Charts::Cyclic); },
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsShutoff::GetPoints,
            owner, &Program::receivedPoints_cyclicTest,
            Qt::BlockingQueuedConnection);

    connect(&t, &CyclicTestsShutoff::SetStartTime,
            owner, &Program::setTimeStart);

    connect(&t, &CyclicTestsShutoff::Results,
            owner, &Program::results_cyclicShutoffTests,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsShutoff::CycleCompleted,
            owner, &Program::cyclicCycleCompleted,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsShutoff::SetMultipleDO,
            owner, &Program::setMultipleDO,
            Qt::QueuedConnection);
}
