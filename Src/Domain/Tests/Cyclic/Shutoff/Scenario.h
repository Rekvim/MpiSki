#pragma once

#include "Domain/Tests/AbstractScenario.h"
#include "Domain/Tests/Context.h"

#include "Params.h"
#include "Result.h"

#include <QVector>

namespace Domain::Tests::Cyclic::Shutoff {

class Runner;
class Analyzer;

class Scenario : public Tests::AbstractScenario
{
    Q_OBJECT

public:
    Scenario(Tests::Context context,
             const Params& params,
             QObject* parent = nullptr);

    ~Scenario() override;

    std::unique_ptr<BaseRunner> createRunner(QObject* parent) override;
    void startAnalyzer() override;
    void onSample(const Measurement::Sample& sample) override;

private slots:
    void onResult(const Result& result);

private:
    Tests::Context m_context;
    Params m_params;
    std::unique_ptr<Analyzer> m_analyzer;
};

}