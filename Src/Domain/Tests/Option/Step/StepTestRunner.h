#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/Option/Step/StepTestParams.h"

class StepTestRunner : public BaseRunner
{
    Q_OBJECT
public:
    StepTestRunner(Mpi& mpi,
                   Registry& reg,
                   const StepTestParams& params,
                   QObject* parent=nullptr)
    : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    StepTestParams m_params;
};
