#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

namespace Domain::Tests::Cyclic::Regulatory {

Scenario::Scenario(Tests::Context context, const Params& params, QObject* parent)
    : Tests::AbstractScenario(parent) , m_context(context) , m_params(params) { }

Scenario::~Scenario() = default;

void Scenario::startAnalyzer()
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

std::unique_ptr<BaseRunner> Scenario::createRunner(QObject* parent)
{
    const bool normalOpen =
        m_context.config.safePosition == SafePosition::NormallyOpen;

    auto runner = std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        m_params,
        parent
        );

    connect(runner.get(), &Runner::cycleCompleted,
            this, &Scenario::cyclicCycleCompleted,
            Qt::BlockingQueuedConnection);

    connect(runner.get(), &Runner::result,
            this, &Scenario::onResult,
            Qt::QueuedConnection);

    return runner;
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