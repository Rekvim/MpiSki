#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

namespace Domain::Tests::Cyclic::Regulatory {

Scenario::Scenario(Tests::Context context, const Params& params, QObject* parent)
    : Tests::AbstractScenario(parent) , m_context(context) , m_params(params) { }

Scenario::~Scenario() = default;

void Scenario::beforeStart()
{
    m_analyzer = std::make_unique<Analyzer>();
    m_analyzer->configure(m_params);
    m_analyzer->start();
}

void Scenario::onSample(const Measurement::Sample& sample)
{
    if (m_analyzer)
        m_analyzer->onSample(sample);
}

std::unique_ptr<BaseRunner> Scenario::createRunner()
{
    const bool normalOpen = m_context.config.safePosition == SafePosition::NormallyOpen;

    return std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        m_params,
        this
    );
}

void Scenario::afterRunnerCreated(BaseRunner& baseRunner)
{
    auto& runner = static_cast<Runner&>(baseRunner);

    connect(&runner, &Runner::cycleCompleted,
            this, &Scenario::cyclicCycleCompleted,
            Qt::BlockingQueuedConnection);

    connect(&runner, &Runner::result,
            this, &Scenario::onResult,
            Qt::QueuedConnection);
}

void Scenario::onResult()
{
    if (!m_analyzer)
        return;

    m_analyzer->finish();

    auto result = m_analyzer->result();

    m_context.telemetry.testСyclicRegulatory = result;

    emit cyclicRegulatoryResultUpdated(result);
    emit telemetryUpdated(m_context.telemetry);
}

}