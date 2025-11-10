#include "MainTestRunner.h"
#include "./Src/Tests/MainTest.h"
#include "./Program.h"

RunnerConfig MainTestRunner::buildConfig() {
    MainTestSettings::TestParameters p{};
    emit getParameters_mainTest(p);
    if (p.delay == 0) {
        return {};
    }

    const quint64 delay = p.delay;
    const quint64 response = p.response;
    const quint64 pn = static_cast<quint64>(p.pointNumbers * p.delay / p.response);
    const quint64 totalMs =
        10000ULL + delay + (pn + 1) * response + delay + (pn + 1) * response + 10000ULL;

    // Коррекция границ DAC по твоим правилам
    p.dac_min = qMax(m_mpi.GetDAC()->GetRawFromValue(p.signal_min), m_mpi.GetDac_Min());
    p.dac_max = qMin(m_mpi.GetDAC()->GetRawFromValue(p.signal_max), m_mpi.GetDac_Max());

    auto* worker = new MainTest;
    worker->SetParameters(p);

    RunnerConfig cfg;
    cfg.worker       = worker;
    cfg.totalMs      = totalMs;
    cfg.chartToClear = /* например */ 0; // Charts::Task (подставь твой enum)
    return cfg;
}

void MainTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<MainTest&>(base);
    auto owner = qobject_cast<Program*>(parent());
    Q_ASSERT(owner);

    connect(&t, &MainTest::EndTest,
            owner, &Program::endTest);
    connect(&t, &MainTest::EndTest,
            owner, &Program::mainTestFinished);

    connect(&t, &MainTest::UpdateGraph,
            owner, &Program::updateCharts_mainTest);

    connect(&t, &MainTest::DublSeries,
            owner, [owner]{ emit owner->dublSeries(); });

    connect(&t, &MainTest::GetPoints,
            owner, &Program::receivedPoints_mainTest,
            Qt::BlockingQueuedConnection);

    connect(&t, &MainTest::AddRegression,
            owner, &Program::addRegression);

    connect(&t, &MainTest::AddFriction,
            owner, &Program::addFriction);

    connect(&t, &MainTest::Results,
            owner, &Program::results_mainTest);

    connect(&t, &MainTest::ShowDots,
            owner, [owner](bool v){ emit owner->showDots(v); });

    connect(&t, &MainTest::ClearGraph, owner, [owner]{
        emit owner->clearPoints(Charts::Task);
        emit owner->clearPoints(Charts::Pressure);
        emit owner->clearPoints(Charts::Friction);
        emit owner->setRegressionEnable(false);
    });
}
