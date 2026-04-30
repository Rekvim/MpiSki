#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

namespace Domain::Tests::Cyclic::Shutoff {

Scenario::Scenario(Tests::Context context,
                   const Params& params,
                   QObject* parent)
    : Tests::AbstractScenario(parent)
    , m_context(context)
    , m_params(params)
{
}

Scenario::~Scenario() = default;

void Scenario::beforeStart()
{
    m_analyzer = std::make_unique<Analyzer>();

    m_analyzer->configure(m_params.numCycles, 2);
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

    connect(&runner, &Runner::GetDI,
            this, &Scenario::diRequested,
            Qt::DirectConnection);

    connect(&runner, &Runner::GetDO,
            this, &Scenario::doRequested,
            Qt::DirectConnection);

    connect(&runner, &Runner::result,
            this, &Scenario::onResult,
            Qt::QueuedConnection);
}

void Scenario::onResult(const Result& result)
{
    if (m_analyzer)
        m_analyzer->finish();

    if (!m_context.telemetry.testСyclicShutoff)
        m_context.telemetry.testСyclicShutoff.emplace();

    auto& dst = *m_context.telemetry.testСyclicShutoff;

    dst.numCycles = result.numCycles;
    dst.doOnCounts = result.doOnCounts;
    dst.doOffCounts = result.doOffCounts;

    dst.switch3to0Count = result.switch3to0Count;
    dst.switch0to3Count = result.switch0to3Count;

    emit cyclicShutoffResultUpdated(dst);
    emit telemetryUpdated(m_context.telemetry);
}

}