#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/CyclicRegulatory/CyclicTestParams.h"

class CyclicShutoffRunner : public BaseRunner {
    Q_OBJECT
public:
    CyclicShutoffRunner(Mpi& mpi,
                   Registry& reg,
                   const CyclicTestParams& params,
                   QObject* parent=nullptr)
        : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    CyclicTestParams m_params;
};
