#pragma once

#include "Src/Domain/Tests/BaseRunner.h"

class StrokeTestRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};
