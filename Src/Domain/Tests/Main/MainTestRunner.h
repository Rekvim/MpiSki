#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/Main/MainTestParams.h"

class MainTestRunner : public BaseRunner {
    Q_OBJECT
public:
    MainTestRunner(Mpi& mpi,
                   Registry& reg,
                   const MainTestParams& params,
                   QObject* parent=nullptr)
    : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    MainTestParams m_params;
};
