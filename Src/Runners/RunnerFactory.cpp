#include "RunnerFactory.h"
#include "MainTestRunner.h"
#include "StepTestRunner.h"
#include "OptionResolutionRunner.h"
#include "OptionResponseRunner.h"
#include "StrokeTestRunner.h"
#include "CyclicRegulatoryRunner.h"


std::unique_ptr<ITestRunner>
RunnerFactory::create(RunnerKind kind, Mpi& mpi, Registry& reg, QObject* parent)
{
    switch (kind) {
    case RunnerKind::Main: return std::make_unique<MainTestRunner>(mpi, reg, parent);
    case RunnerKind::Step: return std::make_unique<StepTestRunner>(mpi, reg, parent);
    case RunnerKind::Resolution: return std::make_unique<OptionResolutionRunner>(mpi, reg, parent);
    case RunnerKind::Response: return std::make_unique<OptionResponseRunner>(mpi, reg, parent);
    case RunnerKind::Stroke: return std::make_unique<StrokeTestRunner>(mpi, reg, parent);
    case RunnerKind::CyclicReg:return std::make_unique<CyclicRegulatoryRunner>(mpi, reg, parent);

    default: return {};
    }
}
