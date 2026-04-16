#include "StepTestRunner.h"
#include "Src/Domain/Program.h"

static QVector<quint16> buildSequence(const StepTestParams& p,
                                      Mpi& mpi, bool normalOpen)
{
    QVector<quint16> seq;
    auto rawFromPct = [&](quint16 pct){
        const qreal cur = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        return mpi.dac()->rawFromValue(cur);
    };

    const qreal start = 4.0;
    const qreal end = 20.0;

    seq.push_back(mpi.dac()->rawFromValue(start));
    for (auto v : p.points) seq.push_back(rawFromPct(v));
    seq.push_back(mpi.dac()->rawFromValue(end));
    for (auto it = p.points.crbegin(); it != p.points.crend(); ++it)
        seq.push_back(rawFromPct(*it));

    seq.push_back(mpi.dac()->rawFromValue(start));
    return seq;
}

RunnerConfig StepTestRunner::buildConfig() {
    const auto& p = m_params;

    if (p.points.empty()) return {};

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 delay = static_cast<quint64>(p.delay);
    const quint64 N_values = 3 + 2 * P;
    const quint64 totalMs = 10000ULL + N_values * delay;

    auto worker = std::make_unique<StepTest>();
    StepTest::Task task;
    task.delay = p.delay;

    const bool normalOpen = isNormallyOpen();
    task.value = buildSequence(p, m_mpi, normalOpen);
    worker->setTask(task);
    worker->setTValue(p.testValue);

    return makeConfig(std::move(worker), totalMs, Charts::Step);
}

void StepTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<StepTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &StepTest::getPoints,
            owner, &Program::receivedPoints_stepTest,
            Qt::BlockingQueuedConnection);

    connect(&t, &StepTest::results,
            owner, &Program::results_stepTest,
            Qt::QueuedConnection);
}
