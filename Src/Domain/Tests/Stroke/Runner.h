#pragma once

#include "Domain/Tests/BaseRunner.h"

namespace Domain::Tests::Stroke {
    class Runner : public BaseRunner {
        Q_OBJECT
    public:
        using BaseRunner::BaseRunner;

    protected:
        RunnerConfig buildConfig() override;
        void wireSpecificSignals(Test& t) override;
    };
}