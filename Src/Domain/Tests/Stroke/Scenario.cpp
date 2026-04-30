#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

#include <QTime>

namespace Domain::Tests::Stroke {

Scenario::Scenario(Tests::Context context,
                   QObject* parent)
    : Tests::AbstractScenario(parent)
    , m_context(context)
{
}

Scenario::~Scenario() = default;

void Scenario::beforeStart()
{
    m_analyzer = std::make_unique<Analyzer>();

    Analyzer::Config cfg;
    cfg.normalClosed =
        m_context.config.safePosition == SafePosition::NormallyClosed;

    m_analyzer->setConfig(cfg);
    m_analyzer->start();
}

void Scenario::onSample(const Measurement::Sample& sample)
{
    if (m_analyzer)
        m_analyzer->onSample(sample);
}

std::unique_ptr<BaseRunner> Scenario::createRunner()
{
    const bool normalOpen =
        m_context.config.safePosition == SafePosition::NormallyOpen;

    return std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        this
    );
}

void Scenario::afterRunnerCreated(BaseRunner& baseRunner)
{
    auto& runner = static_cast<Runner&>(baseRunner);

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

    result.timeForwardMs =
        QTime(0, 0).addMSecs(result.forwardTimeMs).toString("mm:ss.zzz");

    result.timeBackwardMs =
        QTime(0, 0).addMSecs(result.backwardTimeMs).toString("mm:ss.zzz");

    m_context.telemetry.testStroke = result;

    emit strokeResultUpdated(result);
    emit telemetryUpdated(m_context.telemetry);
}

}