#pragma once

#include "Domain/Tests/AbstractScenario.h"
#include "Domain/Tests/Context.h"
#include "Domain/Tests/Option/Params.h"

namespace Domain::Tests::Option::Resolution {

class Runner;

class Scenario : public Tests::AbstractScenario
{
    Q_OBJECT

public:
    Scenario(Tests::Context context,
             const Option::Params& params,
             QObject* parent = nullptr);

    ~Scenario() override;

    std::unique_ptr<BaseRunner> createRunner(QObject* parent) override;

private:
    Tests::Context m_context;
    Option::Params m_params;
};

}