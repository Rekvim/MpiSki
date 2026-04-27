#pragma once

#include "Domain/Tests/AbstractScenario.h"
#include "Domain/Tests/Context.h"

namespace Domain::Tests::Stroke {
    class Runner;
    class Analyzer;

    class Scenario : public Tests::AbstractScenario
    {
        Q_OBJECT

    public:
        Scenario(Tests::Context context,
                 QObject* parent = nullptr);

        ~Scenario() override;

        std::unique_ptr<BaseRunner> createRunner(QObject* parent) override;
        void startAnalyzer() override;
        void onSample(const Measurement::Sample& sample) override;

    signals:
        void telemetryUpdated(const Telemetry& telemetry);

    private slots:
        void onResult();

    private:
        Tests::Context m_context;
        std::unique_ptr<Analyzer> m_analyzer;
    };
}