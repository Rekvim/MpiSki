#include "CyclicShutoffRunner.h"
#include "./Program.h"

static QVector<quint16> makeRawValues(const QVector<qreal>& seq, Mpi& mpi, bool normalOpen) {
    QVector<quint16> raw; raw.reserve(seq.size());
    for (qreal pct : seq) {
        const qreal cur = 16.0 * (normalOpen ? (100.0 - pct) : pct) / 100.0 + 4.0;
        raw.push_back(mpi.dac()->rawFromValue(cur));
    }
    return raw;
}

RunnerConfig CyclicShutoffRunner::buildConfig() {
    const auto& p = m_params;

    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    if (p.testType != p.Type::Shutoff) return {};

    const bool normalOpen = (m_reg.getValveInfo()->safePosition != 0);

    const quint16 rawSafe = m_mpi.dac()->rawFromValue(4.0);          // <-- как в Option*Runner
    const auto rawCycle   = makeRawValues(p.offSeqValues, m_mpi, normalOpen);

    CyclicTestsShutoff::Task task;
    task.delayMsecs = p.shutoff_delayMs;
    task.holdMsecs  = p.shutoff_holdMs;
    task.cycles     = p.shutoff_numCycles;
    task.doMask     = QVector<bool>(p.shutoff_DO.begin(), p.shutoff_DO.end());

    task.values.clear();
    task.values.reserve(2 + static_cast<int>(rawCycle.size()) * p.shutoff_numCycles);

    // Стартуем из safe (0% для NC, 100% для NO)
    task.values.push_back(rawSafe);

    // Дальше выполняем циклы, но без мгновенных дублей
    for (int c = 0; c < p.shutoff_numCycles; ++c) {
        if (!rawCycle.isEmpty() && task.values.last() == rawCycle.first()) {
            task.values += rawCycle.mid(1);
        } else {
            task.values += rawCycle;
        }
    }

    // Возвращаемся в safe в конце (если уже не там)
    if (!task.values.isEmpty() && task.values.last() != rawSafe) {
        task.values.push_back(rawSafe);
    }

    auto* worker = new CyclicTestsShutoff;
    worker->SetTask(task);

    const quint64 steps   = static_cast<quint64>(task.values.size());
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

    // GetPoints больше НЕ нужен для подсчёта DI — можешь оставить, если рисование требует.
    // connect(&t, &CyclicTestsShutoff::GetPoints,
    //         owner, &Program::receivedPoints_cyclicTest,
    //         Qt::BlockingQueuedConnection);

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

    // НОВОЕ: Blocking-опрос DI/DO из воркера
    connect(&t, &CyclicTestsShutoff::GetDI,
            owner, [owner](quint8& di){ di = owner->getDIStatus(); },
            Qt::BlockingQueuedConnection);

    connect(&t, &CyclicTestsShutoff::GetDO,
            owner, [owner](quint8& m){ m = owner->getDOStatus(); },
            Qt::BlockingQueuedConnection);
}
