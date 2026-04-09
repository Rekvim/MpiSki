#include "StrokeTestRunner.h"
#include "Src/Tests/StrokeTest.h"
#include "Src/Domain/Program.h"

RunnerConfig StrokeTestRunner::buildConfig()
{
    auto worker = std::make_unique<StrokeTest>();

    return makeConfig(std::move(worker), 0, Charts::Stroke);
}

void StrokeTestRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<StrokeTest&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &StrokeTest::Results,
            owner, &Program::results_strokeTest,
            Qt::QueuedConnection);
}
