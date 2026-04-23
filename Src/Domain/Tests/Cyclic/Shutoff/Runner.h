#pragma once

#include "Domain/Tests/BaseRunner.h"
#include "Params.h"

namespace Domain::Tests::Cyclic::Shutoff {
    class Runner : public BaseRunner {
        Q_OBJECT
    public:
        Runner(Mpi::Device& device, bool normalOpen, const Params& params,
               QObject* parent = nullptr)
            : BaseRunner(device, normalOpen, parent), m_params(params) {}

    protected:
        RunnerConfig buildConfig() override;
        void wireSpecificSignals(Test& t) override;

    private:
        Params m_params;
    };
}