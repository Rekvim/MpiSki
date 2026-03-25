#include "TestController.h"

TestController::TestController(QObject* parent)
    : QObject(parent)
{
}

void TestController::setProgram(Program* program)
{
    m_program = program;
}

void TestController::setState(TestState s)
{
    if (m_state == s)
        return;

    m_state = s;
    emit stateChanged(s);
}

void TestController::runMainTest(const MainTestSettings::TestParameters& params)
{
    run([this, params]{
        emit startMainRequested(params);
    });
}

void TestController::runStrokeTest()
{
    run([this]{
        emit startStrokeRequested();
    });
}

void TestController::runResponseTest(const OtherTestSettings::TestParameters& params)
{
    run([this, params]{
        emit startResponseRequested(params);
    });
}

void TestController::runResolutionTest(const OtherTestSettings::TestParameters& params)
{
    run([this, params]{
        emit startResolutionRequested(params);
    });
}

void TestController::runStepTest(const StepTestSettings::TestParameters& params)
{
    run([this, params]{
        emit startStepRequested(params);
    });
}

void TestController::runCyclicTest(const CyclicTestSettings::TestParameters& params)
{
    run([this, params]{
        emit startCyclicRequested(params);
    });
}

void TestController::run(std::function<void()> start)
{
    setState(TestState::Starting);

    start();

    setState(TestState::Running);
    emit testStarted();
}

void TestController::finish()
{
    if (m_state == TestState::Running)
    {
        setState(TestState::Finished);
        emit testFinished();
    }
}

void TestController::stop()
{
    if (!m_program)
        return;

    m_program->terminateTest();

    setState(TestState::Canceled);
    emit testFinished();
}
