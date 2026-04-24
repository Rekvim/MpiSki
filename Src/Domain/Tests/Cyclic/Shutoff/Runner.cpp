#include "Runner.h"
#include "Algorithm.h"

#include "Domain/Program.h"
#include "Utils/SignalUtils.h"

namespace Domain::Tests::Cyclic::Shutoff {
    RunnerConfig Runner::buildConfig()
    {
        const auto& p = m_params;

        const auto rawCycle = SignalUtils::makeRawValues(p.sequence, m_device, m_normalOpen);
        if (rawCycle.isEmpty() || p.numCycles <= 0) return {};

        Algorithm::Task task;
        task.delayMsecs = p.delayMs;
        task.holdMsecs = p.holdMs;
        task.cycles = p.numCycles;
        task.doMask = QVector<bool>(p.DO.begin(), p.DO.end());

        task.values = rawCycle;
        auto worker = std::make_unique<Algorithm>();

        worker->setTask(task);

        const quint64 stepsPerCycle = static_cast<quint64>(rawCycle.size());
        const quint64 totalSteps = stepsPerCycle * static_cast<quint64>(task.cycles);
        const quint64 totalMs = totalSteps * static_cast<quint64>(task.delayMsecs + task.holdMsecs);

        return makeConfig(std::move(worker), totalMs, Widgets::Chart::ChartType::Cyclic);
    }

    void Runner::wireSpecificSignals(Test& base) {
        auto& t = static_cast<Algorithm&>(base);
        auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

        connect(&t, &Algorithm::results,
                owner, &Program::results_cyclicShutoffTests,
                Qt::QueuedConnection);

        connect(&t, &Algorithm::cycleCompleted,
                owner, &Program::cyclicCycleCompleted,
                Qt::QueuedConnection);

        connect(&t, &Algorithm::SetMultipleDO,
                owner, &Program::setMultipleDO,
                Qt::QueuedConnection);

        // НОВОЕ: Blocking-опрос DI/DO из воркера
        connect(&t, &Algorithm::GetDI,
                owner, [owner](quint8& di){ di = owner->getDIStatus(); },
                Qt::DirectConnection);

        connect(&t, &Algorithm::GetDO,
                owner, [owner](quint8& m){ m = owner->getDOStatus(); },
                Qt::DirectConnection);
    }
}