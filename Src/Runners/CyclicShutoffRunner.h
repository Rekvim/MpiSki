#ifndef CYCLICSHUTOFFRUNNER_H
#define CYCLICSHUTOFFRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./CyclicTestSettings.h"
#include "./Src/Tests/CyclicTestsShutoff.h"

class CyclicShutoffRunner final : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;
    void setParameters(const CyclicTestSettings::TestParameters& p) { m_params = p; }
private:
    CyclicTestSettings::TestParameters m_params;
protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& base) override;
};

#endif // CYCLICSHUTOFFRUNNER_H
