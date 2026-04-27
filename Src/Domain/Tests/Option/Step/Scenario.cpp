#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

#include <QDebug>

namespace Domain::Tests::Option::Step {
    Scenario::Scenario(Tests::Context context, const Params& params, QObject* parent)
            : Tests::AbstractScenario(parent), m_context(context), m_params(params) {};

    Scenario::~Scenario() = default;

    void Scenario::startAnalyzer()
    {
        m_analyzer = std::make_unique<Analyzer>();
        m_analyzer->setConfig({m_params.testValue});
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

        connect(runner.get(), &Runner::points,
                this, [this](QVector<QVector<QPointF>>& points) {
                    emit pointsRequested(
                        points, Widgets::Chart::ChartType::Step
                        );
                }, Qt::DirectConnection);

        connect(runner.get(), &Runner::results,
                this, &Scenario::onResults,
                Qt::QueuedConnection);

        return runner;
    }

    void Scenario::onResults(const Result& result)
    {
        if (m_analyzer) {
            m_analyzer->finish();

            const auto& analyzerResult = m_analyzer->result();

            qDebug() << "\n===== STEP TEST ANALYZER =====";

            for (const auto& r : analyzerResult.steps) {
                qDebug() << "from:" << r.from
                         << "to:" << r.to
                         << "T:" << r.T_value
                         << "overshoot:" << r.overshoot;
            }

            qDebug() << "=============================\n";
        }

        m_context.telemetry.testStep = result;

        emit stepResultUpdated(result);
        emit telemetryUpdated(m_context.telemetry);
    }
}