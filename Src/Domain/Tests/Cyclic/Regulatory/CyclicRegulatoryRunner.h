#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/Cyclic/CyclicTestParams.h"

class CyclicRegulatoryRunner final : public BaseRunner {
    Q_OBJECT
public:
    CyclicRegulatoryRunner(Mpi& mpi,
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
