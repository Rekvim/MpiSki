#pragma once

#include <memory>

#include "Domain/Tests/TestScenario.h"
#include "Domain/TestContext.h"

namespace Domain::Tests::Main {
struct Params;
}

namespace Domain::Tests::Option {
struct Params;
}

namespace Domain::Tests::Option::Step {
struct Params;
}

namespace Domain::Tests::Cyclic::Regulatory {
struct Params;
}

namespace Domain::Tests::Cyclic::Shutoff {
struct Params;
}

namespace Domain::Tests {

class TestScenarioFactory
{
public:
    static std::unique_ptr<TestScenario> createStroke(
        TestContext context,
        QObject* parent = nullptr);

    static std::unique_ptr<TestScenario> createMain(
        TestContext context,
        const Main::Params& params,
        QObject* parent = nullptr);

    static std::unique_ptr<TestScenario> createStep(
        TestContext context,
        const Option::Step::Params& params,
        QObject* parent = nullptr);

    static std::unique_ptr<TestScenario> createResponse(
        TestContext context,
        const Option::Params& params,
        QObject* parent = nullptr);

    static std::unique_ptr<TestScenario> createResolution(
        TestContext context,
        const Option::Params& params,
        QObject* parent = nullptr);

    static std::unique_ptr<TestScenario> createCyclicRegulatory(
        TestContext context,
        const Cyclic::Regulatory::Params& params,
        QObject* parent = nullptr);

    static std::unique_ptr<TestScenario> createCyclicShutoff(
        TestContext context,
        const Cyclic::Shutoff::Params& params,
        QObject* parent = nullptr);
};

}