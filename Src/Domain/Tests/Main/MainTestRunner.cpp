#include "MainTestRunner.h"
#include "Src/Domain/Tests/Main/MainTest.h"
#include "Src/Domain/Program.h"

RunnerConfig MainTestRunner::buildConfig() {
    auto p = m_params;
    if (p.delay == 0) return {};

    const quint64 N = static_cast<quint64>(p.pointNumbers);
    const quint64 upMs = p.delay + N * p.response;
    const quint64 downMs = p.delay + N * p.response;
    const quint64 totalMs = 10000ULL + upMs + downMs + 10000ULL;

    p.dac_min = qMax(m_mpi.dac()->rawFromValue(p.signal_min), m_mpi.dacMin());
    p.dac_max = qMin(m_mpi.dac()->rawFromValue(p.signal_max), m_mpi.dacMax());

    auto worker = std::make_unique<MainTest>();
    worker->setParameters(p);

    return makeConfig(std::move(worker), totalMs, Charts::Task);
}

void MainTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<MainTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &MainTest::getPoints,
            owner, &Program::receivedPoints_mainTest,
            Qt::BlockingQueuedConnection);

    connect(&t, &MainTest::dublSeries,
            owner, [owner]{ emit owner->duplicateMainChartsSeries(); });

    connect(&t, &MainTest::addRegression,
            owner, &Program::addRegression,
            Qt::QueuedConnection);

    connect(&t, &MainTest::addFriction,
            owner, &Program::addFriction,
            Qt::QueuedConnection);

    connect(&t, &MainTest::results,
            owner, &Program::results_mainTest,
            Qt::QueuedConnection);

    // connect(&t, &MainTest::clearGraph, owner, [owner]{
    //     emit owner->clearPoints(Charts::Task);
    //     emit owner->clearPoints(Charts::Pressure);
    //     emit owner->clearPoints(Charts::Friction);
    //     emit owner->setRegressionEnable(false);
    // });
}
