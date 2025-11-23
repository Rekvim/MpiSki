#ifndef STEPTESTRUNNER_H
#define STEPTESTRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./Src/Tests/StepTest.h"
#include "./StepTestSettings.h"

class StepTestRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

signals:
    // void getParameters_stepTest(OtherTestSettings::TestParameters&);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};


#endif // STEPTESTRUNNER_H
