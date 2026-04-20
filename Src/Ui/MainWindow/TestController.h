#pragma once

#include <QObject>
#include <functional>
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
    void runMainTest(const Domain::Tests::Main::Params& params);
    void runStrokeTest();
    void runResponseTest(const OptionTestParams& params);
    void runResolutionTest(const OptionTestParams& params);
    void runStepTest(const StepTestParams& params);
    void runCyclicTest(const Domain::Tests::Cyclic::Params& params);

    void stop();

signals:
    void stateChanged(TestState state);

    void startMainRequested(const Domain::Tests::Main::Params& params);
    void startStrokeRequested();
    void startResponseRequested(const OptionTestParams& params);
    void startResolutionRequested(const OptionTestParams& params);
    void startStepRequested(const StepTestParams& params);
    void startCyclicRequested(const Domain::Tests::Cyclic::Params& params);

private slots:
    void onProgramActuallyStarted();
    void onProgramFinished();

private:
    void run(const std::function<void()>& start);
    void setState(TestState s);

private:
    Program* m_program = nullptr;
    TestState m_state = TestState::Idle;
    bool m_stopRequested = false;
};