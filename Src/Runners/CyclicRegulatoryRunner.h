#ifndef CYCLICREGULATORYRUNNER_H
#define CYCLICREGULATORYRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./Src/Ui/TestSettings/CyclicTestSettings.h"

class CyclicRegulatoryRunner final : public BaseRunner {
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


#endif // CYCLICREGULATORYRUNNER_H
