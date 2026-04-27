#include "TestScenarioFactory.h"

#include "Domain/Tests/Stroke/Scenario.h"
#include "Domain/Tests/Main/Scenario.h"
#include "Domain/Tests/Option/Step/Scenario.h"
#include "Domain/Tests/Option/Response/Scenario.h"
#include "Domain/Tests/Option/Resolution/Scenario.h"
#include "Domain/Tests/Cyclic/Regulatory/Scenario.h"
#include "Domain/Tests/Cyclic/Shutoff/Scenario.h"

namespace Domain::Tests {
    std::unique_ptr<TestScenario>
    TestScenarioFactory::createStroke(TestContext context, QObject* parent) {
        return std::make_unique<Stroke::Scenario>(
            context,
            parent
        );
    }

    std::unique_ptr<TestScenario>
    TestScenarioFactory::createMain(TestContext context, const Main::Params& params, QObject* parent) {
        return std::make_unique<Main::Scenario>(
            context,
            params,
            parent
        );
    }

    std::unique_ptr<TestScenario>
    TestScenarioFactory::createStep(TestContext context, const Option::Step::Params& params, QObject* parent) {
        return std::make_unique<Option::Step::Scenario>(
            context,
            params,
            parent
        );
    }

    std::unique_ptr<TestScenario>
    TestScenarioFactory::createResponse(TestContext context, const Option::Params& params, QObject* parent)
    {
        return std::make_unique<Option::Response::Scenario>(
            context,
            params,
            parent
        );
    }

    std::unique_ptr<TestScenario>
    TestScenarioFactory::createResolution(TestContext context, const Option::Params& params, QObject* parent) {
        return std::make_unique<Option::Resolution::Scenario>(
            context,
            params,
            parent
        );
    }

    std::unique_ptr<TestScenario>
    TestScenarioFactory::createCyclicRegulatory(TestContext context, const Cyclic::Regulatory::Params& params, QObject* parent) {
        return std::make_unique<Cyclic::Regulatory::Scenario>(
            context,
            params,
            parent
        );
    }

    std::unique_ptr<TestScenario>
    TestScenarioFactory::createCyclicShutoff(TestContext context, const Cyclic::Shutoff::Params& params, QObject* parent) {
        return std::make_unique<Cyclic::Shutoff::Scenario>(
            context,
            params,
            parent
        );
    }
}