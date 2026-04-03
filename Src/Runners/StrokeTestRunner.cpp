#include "StrokeTestRunner.h"
#include "Src/Tests/StrokeTest.h"
#include "Src/Domain/Program.h"

RunnerConfig StrokeTestRunner::buildConfig()
{
    auto* worker = new StrokeTest;

    RunnerConfig rc;
    rc.worker = worker;
    rc.totalMs = 0;
    rc.chartToClear = static_cast<int>(Charts::Stroke);

    return rc;
}

void StrokeTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<StrokeTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &StrokeTest::Results,
            owner, &Program::results_strokeTest,
            Qt::QueuedConnection);
}
