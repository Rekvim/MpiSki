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
    m_analyzer->start();
    m_analyzer->configure(m_params);
}
void Scenario::onSample(const Measurement::Sample& sample)
{
    if (m_analyzer)
        m_analyzer->onSample(sample);
}

void Scenario::updateChart(const Measurement::Sample& s) {
    emitTimePoints(Widgets::Chart::ChartType::Cyclic, s, s.testTime);
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
            Qt::DirectConnection);

    connect(&runner, &Runner::result,
            this, &Scenario::onResult,
            Qt::DirectConnection);
}

void Scenario::onResult()
{
    if (!m_analyzer)
        return;

    m_analyzer->finish();

    auto result = m_analyzer->result();

    QStringList parts;

    for (qreal v : m_params.sequence) parts << QString::number(v);

    result.sequence = parts.join('-');
    result.numCycles = m_params.numCycles;
    result.totalTimeSec = static_cast<quint64>(m_params.sequence.size())
                          * m_params.numCycles
                          * static_cast<quint64>(m_params.delayMs + m_params.holdMs) / 1000.0;

    m_context.telemetry.testСyclicRegulatory = result;

    emit cyclicRegulatoryResultUpdated(result);
    emit telemetryUpdated(m_context.telemetry);
}

}