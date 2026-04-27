#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

namespace Domain::Tests::Cyclic::Shutoff {

Scenario::Scenario(Tests::TestContext context,
                   const Params& params,
                   QObject* parent)
    : Tests::TestScenario(parent)
    , m_context(context)
    , m_params(params)
{
}

Scenario::~Scenario() = default;

void Scenario::startAnalyzer()
{
    m_analyzer = std::make_unique<Analyzer>();

    // У тебя раньше было configure(params.shutoff.numCycles, 2)
    // 2 оставляю как есть, потому что не знаю точный смысл второго параметра.
    m_analyzer->configure(m_params.numCycles, 2);
    m_analyzer->start();
}

void Scenario::onSample(const Measurement::Sample& sample)
{
    if (m_analyzer)
        m_analyzer->onSample(sample);
}

std::unique_ptr<BaseRunner> Scenario::createRunner(QObject* parent)
{
    const bool normalOpen = m_context.config.safePosition == SafePosition::NormallyOpen;

    auto runner = std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        m_params,
        parent
    );

    connect(runner.get(), &Runner::SetMultipleDO,
            this, &Scenario::setMultipleDORequested,
            Qt::BlockingQueuedConnection);

    connect(runner.get(), &Runner::cycleCompleted,
            this, &Scenario::cyclicCycleCompleted,
            Qt::BlockingQueuedConnection);

    connect(runner.get(), &Runner::GetDI,
            this, &Scenario::diRequested,
            Qt::DirectConnection);

    connect(runner.get(), &Runner::GetDO,
            this, &Scenario::doRequested,
            Qt::DirectConnection);

    connect(runner.get(), &Runner::result,
            this, &Scenario::onResult,
            Qt::QueuedConnection);

    return runner;
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

    emit telemetryUpdated(m_context.telemetry);
}

}