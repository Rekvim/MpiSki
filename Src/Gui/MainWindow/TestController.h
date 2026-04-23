#pragma once

#include <QObject>
#include <functional>
#include "Domain/Program.h"

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

    void setProgram(Domain::Program* program);

public slots:
    void runMainTest(const Domain::Tests::Main::Params& params);
    void runStrokeTest();
    void runResponseTest(const Domain::Tests::Option::Params& params);
    void runResolutionTest(const Domain::Tests::Option::Params& params);
    void runStepTest(const Domain::Tests::Option::Step::Params& params);
    void runCyclicTest(const Domain::Tests::Cyclic::Params& params);

    void stop();

signals:
    void stateChanged(TestState state);

    void startMainRequested(const Domain::Tests::Main::Params& params);
    void startStrokeRequested();
    void startResponseRequested(const Domain::Tests::Option::Params& params);
    void startResolutionRequested(const Domain::Tests::Option::Params& params);
    void startStepRequested(const Domain::Tests::Option::Step::Params& params);
    void startCyclicRequested(const Domain::Tests::Cyclic::Params& params);

private slots:
    void onProgramActuallyStarted();
    void onProgramFinished();

private:
    void run(const std::function<void()>& start);
    void setState(TestState s);

private:
    Domain::Program* m_program = nullptr;
    TestState m_state = TestState::Idle;
    bool m_stopRequested = false;
};