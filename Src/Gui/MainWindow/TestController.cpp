#include "TestController.h"

#include "Domain/Program.h"
#include "Domain/Tests/Main/Params.h"
#include "Domain/Tests/Cyclic/Params.h"
#include "Domain/Tests/Option/Step/Params.h"
#include "Domain/Tests/Option/Params.h"

void TestController::setProgram(Domain::Program* program)
{
    if (m_program == program)
        return;

    if (m_program) {
        disconnect(m_program, nullptr, this, nullptr);
    }

    m_program = program;

    if (!m_program)
        return;

    connect(m_program, &Domain::Program::testActuallyStarted,
            this, &TestController::onProgramActuallyStarted);

    connect(m_program, &Domain::Program::testFinished,
            this, &TestController::onProgramFinished);
}

void TestController::setState(TestState s)
{
    if (m_state == s)
        return;

    m_state = s;
    emit stateChanged(s);
}

void TestController::runStrokeTest()
{
    run([this] {
        emit startStrokeRequested();
    });
}

void TestController::runMainTest(const Domain::Tests::Main::Params& params)
{
    run([this, params] {
        emit startMainRequested(params);
    });
}

void TestController::runResponseTest(const Domain::Tests::Option::Params& params)
{
    run([this, params] {
        emit startResponseRequested(params);
    });
}

void TestController::runResolutionTest(const Domain::Tests::Option::Params& params)
{
    run([this, params] {
        emit startResolutionRequested(params);
    });
}

void TestController::runStepTest(const Domain::Tests::Option::Step::Params& params)
{
    run([this, params] {
        emit startStepRequested(params);
    });
}

void TestController::runCyclicTest(const Domain::Tests::Cyclic::Params& params)
{
    run([this, params] {
        emit startCyclicRequested(params);
    });
}

void TestController::run(const std::function<void()>& start)
{
    if (m_state == TestState::Starting || m_state == TestState::Running)
        return;

    m_stopRequested = false;
    setState(TestState::Starting);
    start();
}

void TestController::onProgramActuallyStarted()
{
    setState(TestState::Running);
}

void TestController::onProgramFinished()
{
    if (m_state == TestState::Idle)
        return;

    if (m_stopRequested)
        setState(TestState::Canceled);
    else
        setState(TestState::Finished);
}

void TestController::stop()
{
    if (!m_program)
        return;

    if (m_state != TestState::Starting &&
        m_state != TestState::Running)
        return;

    m_stopRequested = true;
    m_program->terminateTest();
}