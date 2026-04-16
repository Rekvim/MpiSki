#include "CyclicShutoffRunner.h"
#include "Src/Domain/Program.h"
#include "Src/Utils/SignalUtils.h"

RunnerConfig CyclicShutoffRunner::buildConfig()
{
    const auto& p = m_params.shutoff;

    const bool normalOpen = isNormallyOpen();
    const auto rawCycle = SignalUtils::makeRawValues(p.sequence, m_mpi, normalOpen);
    if (rawCycle.isEmpty() || p.numCycles <= 0) return {};

    CyclicTestsShutoff::Task task;
    task.delayMsecs = p.delayMs;
    task.holdMsecs = p.holdMs;
    task.cycles = p.numCycles;
    task.doMask = QVector<bool>(p.DO.begin(), p.DO.end());

    task.values = rawCycle;
    auto worker = std::make_unique<CyclicTestsShutoff>();

    worker->setTask(task);

    const quint64 stepsPerCycle = static_cast<quint64>(rawCycle.size());
    const quint64 totalSteps = stepsPerCycle * static_cast<quint64>(task.cycles);
    const quint64 totalMs = totalSteps * static_cast<quint64>(task.delayMsecs + task.holdMsecs);

    RunnerConfig cfg;
    cfg.worker = std::move(worker);
    cfg.totalMs = totalMs;
    cfg.chartToClear = Charts::Cyclic;
    return makeConfig(std::move(worker), totalMs, Charts::Cyclic);
}

void CyclicShutoffRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<CyclicTestsShutoff&>(base);
    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    connect(&t, &CyclicTestsShutoff::results,
            owner, &Program::results_cyclicShutoffTests,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsShutoff::cycleCompleted,
            owner, &Program::cyclicCycleCompleted,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsShutoff::SetMultipleDO,
            owner, &Program::setMultipleDO,
            Qt::QueuedConnection);

    // НОВОЕ: Blocking-опрос DI/DO из воркера
    connect(&t, &CyclicTestsShutoff::GetDI,
            owner, [owner](quint8& di){ di = owner->getDIStatus(); },
            Qt::DirectConnection);

    connect(&t, &CyclicTestsShutoff::GetDO,
            owner, [owner](quint8& m){ m = owner->getDOStatus(); },
            Qt::DirectConnection);
}
