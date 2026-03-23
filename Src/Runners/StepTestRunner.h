#pragma once

#include "BaseRunner.h"
#include "./Src/Ui/TestSettings/StepTestSettings.h"

class StepTestRunner : public BaseRunner
{
    Q_OBJECT
public:
    StepTestRunner(Mpi& mpi,
                   Registry& reg,
                   const StepTestSettings::TestParameters& params,
                   QObject* parent=nullptr)
        : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    StepTestSettings::TestParameters m_params;
};
