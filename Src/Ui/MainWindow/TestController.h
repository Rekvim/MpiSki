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
    void runMainTest(const MainTestParams& params);
    void runStrokeTest();
    void runResponseTest(const OptionTestParams& params);
    void runResolutionTest(const OptionTestParams& params);
    void runStepTest(const StepTestParams& params);
    void runCyclicTest(const CyclicTestParams& params);

    void stop();

signals:
    void stateChanged(TestState state);

    void startMainRequested(const MainTestParams& params);
    void startStrokeRequested();
    void startResponseRequested(const OptionTestParams& params);
    void startResolutionRequested(const OptionTestParams& params);
    void startStepRequested(const StepTestParams& params);
    void startCyclicRequested(const CyclicTestParams& params);

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