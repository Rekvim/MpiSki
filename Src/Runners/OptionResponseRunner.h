#pragma once

#include "BaseRunner.h"
#include "Src/Ui/TestSettings/OtherTestSettings.h"

class OptionResponseRunner : public BaseRunner
{
    Q_OBJECT
public:
    OptionResponseRunner(Mpi& mpi,
                           Registry& reg,
                           const OtherTestSettings::TestParameters& params,
                           QObject* parent=nullptr)
        : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    OtherTestSettings::TestParameters m_params;
};
