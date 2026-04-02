#pragma once

#include <QObject>
#include "BaseRunner.h"
#include "Src/Domain/TestParams/MainTestParams.h"

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
