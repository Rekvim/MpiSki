#include "CyclicRegulatoryRunner.h"
#include "Src/Domain/Program.h"
#include "CyclicTestsRegulatory.h"
#include "Src/Utils/SignalUtils.h"

RunnerConfig CyclicRegulatoryRunner::buildConfig()
{
    auto p = m_params;

    if (p.testType != p.Type::Regulatory &&
        p.testType != p.Type::Combined)
    {
        return {};
    }

    const bool normalOpen = isNormallyOpen();
    const auto raw = SignalUtils::makeRawValues(p.regSeqValues, m_mpi, normalOpen);

    CyclicTestsRegulatory::Task task;
    task.delayMsecs = p.regulatory_delayMs;
    task.holdMsecs = p.regulatory_holdMs;
    task.sequence = p.regSeqValues;
    task.values = raw;
    task.cycles = p.regulatory_numCycles;

    auto worker = std::make_unique<CyclicTestsRegulatory>();
    worker->SetTask(task);

    const quint64 stepsPerCycle = static_cast<quint64>(raw.size());
    const quint64 totalSteps = stepsPerCycle * p.regulatory_numCycles;
    const quint64 totalMs = totalSteps * (p.regulatory_delayMs + p.regulatory_holdMs);

    return makeConfig(std::move(worker), totalMs, Charts::Cyclic);;
}

void CyclicRegulatoryRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<CyclicTestsRegulatory&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &CyclicTestsRegulatory::StepMeasured,
            owner, &Program::onCyclicStepMeasured,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::Results,
            owner, &Program::results_cyclicRegulatoryTests,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::CycleCompleted,
            owner, &Program::cyclicCycleCompleted,
            Qt::QueuedConnection);
}
