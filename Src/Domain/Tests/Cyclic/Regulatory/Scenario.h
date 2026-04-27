#pragma once

#include "Domain/Tests/TestScenario.h"
#include "Domain/TestContext.h"

#include "Params.h"

namespace Domain::Tests::Cyclic::Regulatory {

class Runner;
class Analyzer;

class Scenario : public Tests::TestScenario
{
    Q_OBJECT

public:
    Scenario(Tests::TestContext context,
             const Params& params,
             QObject* parent = nullptr);

    ~Scenario() override;

    std::unique_ptr<BaseRunner> createRunner(QObject* parent) override;
    void startAnalyzer() override;
    void onSample(const Measurement::Sample& sample) override;

private slots:
    void onResult();

private:
    Tests::TestContext m_context;
    Params m_params;
    std::unique_ptr<Analyzer> m_analyzer;
};

}