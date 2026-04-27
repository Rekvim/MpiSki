#include "Scenario.h"

#include "Runner.h"

namespace Domain::Tests::Option::Response {
    Scenario::Scenario(Tests::TestContext context, const Option::Params& params, QObject* parent)
        : Tests::TestScenario(parent), m_context(context), m_params(params) { }

    Scenario::~Scenario() = default;

    std::unique_ptr<BaseRunner> Scenario::createRunner(QObject* parent)
    {
        const bool normalOpen = m_context.config.safePosition == SafePosition::NormallyOpen;

        return std::make_unique<Runner>(
            m_context.device,
            normalOpen,
            m_params,
            parent
        );
    }
}