#pragma once

#include "BaseRunner.h"
#include "Src/Ui/TestSettings/CyclicTestSettings.h"

class CyclicShutoffRunner : public BaseRunner {
    Q_OBJECT
public:
    CyclicShutoffRunner(Mpi& mpi,
                   Registry& reg,
                   const CyclicTestSettings::TestParameters& params,
                   QObject* parent=nullptr)
        : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    CyclicTestSettings::TestParameters m_params;
};
