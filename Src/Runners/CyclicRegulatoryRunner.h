#ifndef CYCLICREGULATORYRUNNER_H
#define CYCLICREGULATORYRUNNER_H

#pragma once
#include "BaseRunner.h"
#include "./CyclicTestSettings.h"
#include "./Src/Tests/CyclicTestsRegulatory.h"

class CyclicRegulatoryRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

signals:
    void getParameters_cyclicTest(CyclicTestSettings::TestParameters&);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};


#endif // CYCLICREGULATORYRUNNER_H
