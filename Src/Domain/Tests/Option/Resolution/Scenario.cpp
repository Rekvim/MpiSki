#include "Scenario.h"

#include "Runner.h"

namespace Domain::Tests::Option::Resolution {
Scenario::Scenario(Tests::Context context, const Option::Params& params, QObject* parent)
    : Tests::AbstractScenario(parent), m_context(context), m_params(params) { }

std::unique_ptr<BaseRunner> Scenario::createRunner()
{
    const bool normalOpen =
        m_context.config.safePosition == SafePosition::NormallyOpen;

    return std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        m_params,
        this
    );
}
}