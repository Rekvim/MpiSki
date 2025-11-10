#include "CyclicRegulatoryRunner.h"
#include "./Program.h"

static QVector<quint16> makeRawValues(const QVector<quint16>& seq, MPI& mpi, bool normalOpen)
{
    QVector<quint16> raw; raw.reserve(seq.size());
    for (quint16 pct : seq) {
        const qreal cur = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        raw.push_back(mpi.GetDAC()->GetRawFromValue(cur));
    }
    return raw;
}

RunnerConfig CyclicRegulatoryRunner::buildConfig() {
    CyclicTestSettings::TestParameters p{};
    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);
    emit owner->getParameters_cyclicTest(p);

    if (p.testType != CyclicTestSettings::TestParameters::Regulatory) return {};

    const bool normalOpen = (m_reg.getValveInfo()->safePosition != 0);
    const auto raw = makeRawValues(p.regSeqValues, m_mpi, normalOpen);

    CyclicTestsRegulatory::Task task;
    task.delayMsecs = p.regulatory_delayMs;
    task.holdMsecs = p.regulatory_holdMs;
    task.sequence = p.regSeqValues;

    task.values.reserve(p.regulatory_numCycles * raw.size());
    for (int c = 0; c < p.regulatory_numCycles; ++c) task.values += raw;

    auto* worker = new CyclicTestsRegulatory;
    worker->SetTask(task);

    const quint64 steps = static_cast<quint64>(raw.size()) * p.regulatory_numCycles;
    const quint64 totalMs = steps * (p.regulatory_delayMs + p.regulatory_holdMs);

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = totalMs;
    cfg.chartToClear = static_cast<int>(Charts::Cyclic);
    return cfg;
}

void CyclicRegulatoryRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<CyclicTestsRegulatory&>(base);
    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    connect(&t, &CyclicTestsRegulatory::UpdateGraph,
            owner, [owner]{ owner->updateCharts_CyclicTest(Charts::Cyclic); });

    connect(&t, &CyclicTestsRegulatory::GetPoints,
            owner, &Program::receivedPoints_cyclicTest,
            Qt::BlockingQueuedConnection);

    connect(owner, &Program::releaseBlock,
            &t, &CyclicTestsRegulatory::ReleaseBlock);

    connect(&t, &CyclicTestsRegulatory::SetStartTime,
            owner, &Program::setTimeStart);

    connect(&t, &CyclicTestsRegulatory::Results,
            owner, &Program::results_cyclicRegulatoryTests);

    connect(&t, &CyclicTestsRegulatory::CycleCompleted,
            owner, &Program::cyclicCycleCompleted);
}
