#pragma once

#include <QObject>
#include "BaseRunner.h"
#include "./Src/Ui/TestSettings/MainTestSettings.h"

class MainTestRunner : public BaseRunner {
    Q_OBJECT
public:
    MainTestRunner(Mpi& mpi,
                   Registry& reg,
                   const MainTestSettings::TestParameters& params,
                   QObject* parent=nullptr)
    : BaseRunner(mpi, reg, parent), m_params(params) {}

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;

private:
    MainTestSettings::TestParameters m_params;
};
