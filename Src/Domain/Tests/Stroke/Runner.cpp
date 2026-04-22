#include "Runner.h"
#include "Algorithm.h"
#include "Src/Domain/Program.h"

namespace Domain::Tests::Stroke {
RunnerConfig Runner::buildConfig()
{
    auto worker = std::make_unique<Algorithm>();

    return makeConfig(std::move(worker), 0, Widgets::Chart::ChartType::Stroke);
}

void Runner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<Algorithm&>(base);
    auto owner = qobject_cast<Program*>(parent());

    connect(&t, &Algorithm::results,
            owner, &Program::results_strokeTest,
            Qt::QueuedConnection);
}
}