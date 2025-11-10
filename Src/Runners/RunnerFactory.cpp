// Src/Runners/RunnerFactory.cpp
#include "RunnerFactory.h"
#include "MainTestRunner.h"
// #include "StepTestRunner.h"
// #include "StrokeTestRunner.h"
// … остальные

std::unique_ptr<ITestRunner>
RunnerFactory::create(RunnerKind kind, MPI& mpi, Registry& reg, QObject* parent)
{
    switch (kind) {
    case RunnerKind::Main: return std::make_unique<MainTestRunner>(mpi, reg, parent);
    // case RunnerKind::Step:   return std::make_unique<StepTestRunner>(mpi, reg, parent);
    // case RunnerKind::Stroke: return std::make_unique<StrokeTestRunner>(mpi, reg, parent);
    // … остальные
    default: return {};
    }
}
