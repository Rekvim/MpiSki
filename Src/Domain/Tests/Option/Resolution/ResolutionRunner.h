#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/Option/OptionTestParams.h"

class ResolutionRunner : public BaseRunner
{
    Q_OBJECT
public:
    ResolutionRunner(Mpi& mpi,
                           Registry& reg,
                           const OptionTestParams& params,
                           QObject* parent=nullptr)
        : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    OptionTestParams m_params;
};
