#pragma once

#include "BaseRunner.h"
#include "Src/Domain/TestParams/OptionTestParams.h"

class OptionResponseRunner : public BaseRunner
{
    Q_OBJECT
public:
    OptionResponseRunner(Mpi& mpi,
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
