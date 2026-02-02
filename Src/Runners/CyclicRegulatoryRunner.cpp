#include "CyclicRegulatoryRunner.h"
#include "./Program.h"
#include "./Src/Tests/CyclicTestsRegulatory.h"

QVector<quint16> makeRawValues(const QVector<qreal>& seq, Mpi& mpi, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.size());

    for (quint16 pct : seq) {
        const qreal current = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        raw.push_back(mpi.dac()->rawFromValue(current));
    }
    return raw;
}

RunnerConfig CyclicRegulatoryRunner::buildConfig()
{
    const auto& p = m_params;

    if (p.testType != p.Type::Regulatory &&
        p.testType != p.Type::Combined)
    {
        return {};
    }

    const bool normalOpen = (m_reg.valveInfo().safePosition != 0);
    const auto raw = makeRawValues(p.regSeqValues, m_mpi, normalOpen);

    CyclicTestsRegulatory::Task task;
    task.delayMsecs = p.regulatory_delayMs;
    task.holdMsecs = p.regulatory_holdMs;
    task.sequence = p.regSeqValues;
    task.values = raw;
    task.cycles = p.regulatory_numCycles;

    auto* worker = new CyclicTestsRegulatory;
    worker->SetTask(task);

    const quint64 stepsPerCycle = static_cast<quint64>(raw.size());
    const quint64 totalSteps = stepsPerCycle * p.regulatory_numCycles;
    const quint64 totalMs = totalSteps * (p.regulatory_delayMs + p.regulatory_holdMs);

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Cyclic);
    return cfg;
}

void CyclicRegulatoryRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<CyclicTestsRegulatory&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &CyclicTestsRegulatory::UpdateGraph,
            owner, [owner]{ owner->updateCharts_CyclicTest(Charts::Cyclic); },
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::SetStartTime,
            owner, &Program::setTimeStart,
            Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::StepMeasured,
            owner, &Program::onCyclicStepMeasured,
            Qt::QueuedConnection);

    // connect(&t, &CyclicTestsRegulatory::Results,
    //         owner, &Program::results_cyclicRegulatoryTests,
    //         Qt::QueuedConnection);

    connect(&t, &CyclicTestsRegulatory::CycleCompleted,
            owner, &Program::cyclicCycleCompleted,
            Qt::QueuedConnection);
}
