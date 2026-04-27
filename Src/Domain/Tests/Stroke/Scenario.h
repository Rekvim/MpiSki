#pragma once

#include "Domain/Tests/TestScenario.h"
#include "Domain/TestContext.h"

namespace Domain::Tests::Stroke {
    class Runner;
    class Analyzer;

    class Scenario : public Tests::TestScenario
    {
        Q_OBJECT

    public:
        Scenario(Tests::TestContext context,
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
        Tests::TestContext m_context;
        std::unique_ptr<Analyzer> m_analyzer;
    };
}