#include "CyclicRegulatoryRunner.h"
#include "Src/Domain/Program.h"
#include "CyclicTestsRegulatory.h"
#include "Src/Utils/SignalUtils.h"

RunnerConfig CyclicRegulatoryRunner::buildConfig()
{
    auto p = m_params.regulatory;

    const bool normalOpen = isNormallyOpen();
    const auto raw = SignalUtils::makeRawValues(p.sequence, m_mpi, normalOpen);

    CyclicTestsRegulatory::Task task;
    task.delayMsecs = p.delayMs;
    task.holdMsecs = p.holdMs;
    task.sequence = p.sequence;
    task.values = raw;
    task.cycles = p.numCycles;

    auto worker = std::make_unique<CyclicTestsRegulatory>();
    worker->SetTask(task);

    const quint64 stepsPerCycle = static_cast<quint64>(raw.size());
    const quint64 totalSteps = stepsPerCycle * p.numCycles;
    const quint64 totalMs = totalSteps * (p.delayMs + p.holdMs);

    return makeConfig(std::move(worker), totalMs, Charts::Cyclic);;
}

void CyclicRegulatoryRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<CyclicTestsRegulatory&>(base);
    auto owner = qobject_cast<Program*>(parent());

    // connect(&t, &CyclicTestsRegulatory::StepMeasured,
    //         owner, &Program::onCyclicStepMeasured,
    //         Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::Results,
            owner, &Program::results_cyclicRegulatoryTests,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::CycleCompleted,
            owner, &Program::cyclicCycleCompleted,
            Qt::QueuedConnection);
}
