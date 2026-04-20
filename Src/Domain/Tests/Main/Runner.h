#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/Main/Params.h"

namespace Domain::Tests::Main {

class Runner : public BaseRunner {
    Q_OBJECT
public:
    Runner(Mpi& mpi,
                   Registry& reg,
                   const Params& params,
                   QObject* parent=nullptr)
    : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    Params m_params;
};
}
