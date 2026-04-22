#include "Runner.h"
#include "Src/Domain/Program.h"
#include "Algorithm.h"
#include "Src/Utils/SignalUtils.h"

namespace Domain::Tests::Cyclic::Regulatory {
RunnerConfig Runner::buildConfig()
{
    auto p = m_params;

    const bool normalOpen = isNormallyOpen();
    const auto raw = SignalUtils::makeRawValues(p.sequence, m_device, normalOpen);

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

void Runner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<Algorithm&>(base);
    auto owner = qobject_cast<Program*>(parent());

    // connect(&t, &CyclicTestsRegulatory::StepMeasured,
    //         owner, &Program::onCyclicStepMeasured,
    //         Qt::QueuedConnection);

    connect(&t, &Algorithm::results,
            owner, &Program::results_cyclicRegulatoryTests,
            Qt::QueuedConnection);

    connect(&t, &Algorithm::cycleCompleted,
            owner, &Program::cyclicCycleCompleted,
            Qt::QueuedConnection);
}
}