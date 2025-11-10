#include "StrokeTestRunner.h"
#include "./Program.h"

RunnerConfig StrokeTestRunner::buildConfig() {
    auto* worker = new StrokeTest;

    RunnerConfig cfg;
    cfg.worker = worker;
    cfg.totalMs = 0;
    cfg.chartToClear = static_cast<int>(Charts::Stroke);
    return cfg;
}

void StrokeTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<StrokeTest&>(base);
    auto owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);

    connect(&t, &StrokeTest::UpdateGraph,
            owner, &Program::updateCharts_strokeTest);

    connect(&t, &StrokeTest::SetStartTime,
            owner, &Program::setTimeStart);

    connect(&t, &StrokeTest::Results,
            owner, &Program::results_strokeTest);
}
