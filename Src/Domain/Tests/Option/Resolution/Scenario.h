#pragma once

#include "Domain/Tests/TestScenario.h"
#include "Domain/TestContext.h"
#include "Domain/Tests/Option/Params.h"

namespace Domain::Tests::Option::Resolution {

class Runner;

class Scenario : public Tests::TestScenario
{
    Q_OBJECT

public:
    Scenario(Tests::TestContext context,
             const Option::Params& params,
             QObject* parent = nullptr);

    ~Scenario() override;

    std::unique_ptr<BaseRunner> createRunner(QObject* parent) override;

private:
    Tests::TestContext m_context;
    Option::Params m_params;
};

}