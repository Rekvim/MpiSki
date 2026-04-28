#include "Runner.h"
#include "Algorithm.h"

#include "Utils/SignalUtils.h"

namespace Domain::Tests::Cyclic::Regulatory {
RunnerConfig Runner::buildConfig()
{
    auto p = m_params;

    const auto raw = SignalUtils::makeRawValues(p.sequence, m_device, m_normalOpen);

    Algorithm::Task task;
    task.delayMsecs = p.delayMs;
    task.holdMsecs = p.holdMs;
    task.sequence = p.sequence;
    task.values = raw;
    task.cycles = p.numCycles;

    auto worker = std::make_unique<Algorithm>();
    worker->setTask(task);

    const quint64 stepsPerCycle = static_cast<quint64>(raw.size());
    const quint64 totalSteps = stepsPerCycle * p.numCycles;
    const quint64 totalMs = totalSteps * (p.delayMs + p.holdMs);

    return makeConfig(std::move(worker), totalMs, Widgets::Chart::ChartType::Cyclic);;
}

void Runner::wireSpecificSignals(AbstractTestAlgorithm& base) {
    auto& t = static_cast<Algorithm&>(base);

    connect(&t, &Algorithm::result,
            this, &Runner::result,
            Qt::QueuedConnection);

    connect(&t, &Algorithm::cycleCompleted,
            this, &Runner::cycleCompleted,
            Qt::QueuedConnection);
}
}