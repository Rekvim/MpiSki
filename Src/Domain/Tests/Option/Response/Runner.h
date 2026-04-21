#pragma once

#include "Src/Domain/Tests/BaseRunner.h"
#include "Src/Domain/Tests/Option/Params.h"

namespace Domain::Tests::Option::Response {
    class Runner : public BaseRunner
    {
        Q_OBJECT
    public:
        Runner(Domain::Mpi::Device& device,
               Registry& reg,
               const Params& params,
               QObject* parent = nullptr)
            : BaseRunner(device, reg, parent), m_params(params) {}

    protected:
        RunnerConfig buildConfig() override;
        void wireSpecificSignals(Test& t) override;

    private:
        Params m_params;
    };
}
