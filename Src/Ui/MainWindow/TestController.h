#pragma once

#include <QObject>
#include "Src/Domain/Program.h"

enum class TestState {
    Idle,
    Starting,
    Running,
    Finished,
    Canceled
};

class TestController : public QObject
{
    Q_OBJECT

public:
    explicit TestController(QObject* parent = nullptr);

    void setProgram(Program* program);

public slots:

    void runMainTest(const MainTestSettings::TestParameters& params);
    void runStrokeTest();
    void runResponseTest(const OtherTestSettings::TestParameters& params);
    void runResolutionTest(const OtherTestSettings::TestParameters& params);
    void runStepTest(const StepTestSettings::TestParameters& params);
    void runCyclicTest(const CyclicTestSettings::TestParameters& params);

    void finish();
    void stop();

signals:

    void testStarted();
    void testFinished();
    void stateChanged(TestState state);

    void startMainRequested(const MainTestSettings::TestParameters& params);
    void startStrokeRequested();
    void startResponseRequested(const OtherTestSettings::TestParameters& params);
    void startResolutionRequested(const OtherTestSettings::TestParameters& params);
    void startStepRequested(const StepTestSettings::TestParameters& params);
    void startCyclicRequested(const CyclicTestSettings::TestParameters& params);

private:
    void run(std::function<void()> start);
    void setState(TestState s);

    Program* m_program = nullptr;
    TestState m_state = TestState::Idle;
};
