#ifndef PROGRAM_H
#define PROGRAM_H

#pragma once
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "./Src/Mpi/Mpi.h"
#include "OtherTestSettings.h"
#include "StepTestSettings.h"
#include "CyclicTestSettings.h"

#include "Registry.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Src/Runners/AbstractTestRunner.h"

#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/MainTest.h"
#include "./Src/Tests/CyclicTests.h"
#include "./Src/Tests/CyclicTestsRegulatory.h"
#include "./Src/Tests/CyclicTestsShutoff.h"

#include "SelectTests.h"

enum class TextObjects {

    LineEdit_linearSensor,
    LineEdit_linearSensorPercent,
    LineEdit_pressureSensor_1,
    LineEdit_pressureSensor_2,
    LineEdit_pressureSensor_3,
    LineEdit_feedback_4_20mA,
};

enum class Charts {
    Task,
    Pressure,
    Friction,
    Response,
    Resolution,
    Stroke,
    Step,
    Trend,
    Cyclic
};

struct Point
{
    quint8 series_num;
    qreal X;
    qreal Y;
};

class Program : public QObject
{
    Q_OBJECT
public:
    explicit Program(QObject *parent = nullptr);
    void setRegistry(Registry *registry);
    bool isInitialized() const;

signals:

    void errorOccured(const QString&);

    void telemetryUpdated(const TelemetryStore &store);
    void cyclicCycleCompleted(int completedCycles);

    void setText(const TextObjects object, const QString &text);
    void setTextColor(const TextObjects object, const QColor color);
    void setTask(qreal task);
    void setSensorNumber(quint8 num);
    void setButtonInitEnabled(bool enable);
    void setGroupDOVisible(bool visible);
    void setVisible(Charts chart, quint16 series, bool visible);
    void setRegressionEnable(bool enable);

    void setStepResults(const QVector<StepTest::TestResult> &results, quint32 T_value);
    void setButtonsDOChecked(quint8 status);
    void setCheckboxDIChecked(quint8 status);
    void getPoints(QVector<QVector<QPointF>> &points, Charts chart);

    void getPoints_mainTest(QVector<QVector<QPointF>> &points, Charts chart);
    void getPoints_stepTest(QVector<QVector<QPointF>> &points, Charts chart);
    void getPoints_cyclicTest(QVector<QVector<QPointF>> &points, Charts chart);

    void addPoints(Charts chart, QVector<Point> points);
    void clearPoints(Charts chart);

    void stopTheTest();
    void showDots(bool visible);
    void enableSetTask(bool enable);
    void dublSeries();
    void releaseBlock();

    void mainTestFinished();
    void getParameters_mainTest(struct MainTestSettings::TestParameters& parameters);
    void getParameters_stepTest(StepTestSettings::TestParameters &parameters);
    void getParameters_resolutionTest(OtherTestSettings::TestParameters &parameters);
    void getParameters_responseTest(OtherTestSettings::TestParameters &parameters);
    void getParameters_cyclicTest(CyclicTestSettings::TestParameters &parameters);

    void question(QString &title, QString &text, bool &result);

    void testFinished();

    void totalTestTimeMs(quint64 totalMs);

private:
    SelectTests::PatternType m_patternType;

    inline qreal calcPercent(qreal value, bool invert = false) {
        qreal percent = ((value - 4.0) / 16.0) * 100.0;
        percent = qBound<qreal>(0.0, percent, 100.0);
        return invert ? (100.0 - percent) : percent;
    }

    std::unique_ptr<AbstractTestRunner> m_activeRunner;
    template<typename RunnerT>
    void startRunner(std::unique_ptr<RunnerT> r) {
        disposeActiveRunnerAsync();
        connect(r.get(), &AbstractTestRunner::requestClearChart, this, [this](int chart){
            emit clearPoints(static_cast<Charts>(chart));
        });
        connect(r.get(), &AbstractTestRunner::requestSetDAC, this, &Program::setDacRaw);
        connect(this, &Program::releaseBlock, r.get(), &AbstractTestRunner::releaseBlock);
        connect(r.get(), &AbstractTestRunner::totalTestTimeMs, this, &Program::totalTestTimeMs);
        connect(r.get(), &AbstractTestRunner::endTest, this, &Program::endTest);
        connect(this, &Program::stopTheTest, r.get(), &AbstractTestRunner::stop);
        emit setButtonInitEnabled(false);
        m_isTestRunning = true;
        emit enableSetTask(false);

        m_activeRunner = std::move(r);
        m_activeRunner->start();
    }

    void updateCrossingStatus();

    void disposeActiveRunnerAsync();

    // init
    bool connectAndInitDevice();
    bool detectAndReportSensors();
    void waitForDacCycle();
    void measureStartPosition(bool normalClosed);
    void measureStartPositionShutoff(bool normalClosed);

    void measureEndPosition(bool normalClosed);
    void measureEndPositionShutoff(bool normalClosed);

    void calculateAndApplyCoefficients();
    void recordStrokeRange(bool normalClosed);
    void finalizeInitialization();


    QVector<quint16> makeRawValues(const QVector<quint16> &seq, bool normalOpen);
    QString seqToString(const QVector<quint16>& seq);

    Registry *m_registry;

    Mpi m_mpi;
    TelemetryStore m_telemetryStore;
    QTimer *m_diPollTimer = nullptr;
    quint8 m_lastDiStatus = 0;
    QTimer *m_timerSensors;
    QTimer *m_timerDI;

    quint64 m_cyclicStartTimeMs = 0;
    quint64 m_startTime;
    quint64 m_initTime;
    QEventLoop *m_dacEventLoop;

    bool m_isInitialized = false;
    bool m_isCyclicTestRunning = false;
    bool m_isTestRunning = false;
    bool m_isDacStopRequested;
    bool m_shouldWaitForButton = false;
    QVector<bool> m_initialDoStates;
    QVector<bool> m_savedInitialDoStates;

private slots:
    void updateSensors();

public slots:
    void onCyclicStepMeasured(int cycle, int step, bool forward);

    void setMultipleDO(const QVector<bool>& states);

    void setDacRaw(quint16 dac,
                quint32 sleep_ms = 0,
                bool waitForStop = false,
                bool waitForStart = false);

    void initialization();

    void updateCharts_mainTest();
    void updateCharts_strokeTest();
    void updateCharts_optionTest(Charts chart);
    void updateCharts_CyclicTest(Charts chart);

    void results_mainTest(const MainTest::TestResults &results);
    void results_strokeTest(const quint64 forwardTime, const  quint64 backwardTime);
    void results_stepTest(const QVector<StepTest::TestResult> &results, const quint32 T_value);

    void results_cyclicTests(const CyclicTests::TestResults& r);
    void results_cyclicRegulatoryTests(const CyclicTestsRegulatory::TestResults& results);
    void results_cyclicShutoffTests(const CyclicTestsShutoff::TestResults& results);

    void results_cyclicCombinedTests(const CyclicTestsRegulatory::TestResults& regulatoryResults,
                                     const CyclicTestsShutoff::TestResults& shutoffResults);

    void setInitDOStates(const QVector<bool> &states);
    void setPattern(SelectTests::PatternType pattern) { m_patternType = pattern; }

    void addRegression(const QVector<QPointF> &points);
    void addFriction(const QVector<QPointF> &points);

    void receivedPoints_mainTest(QVector<QVector<QPointF>> &points);
    void receivedPoints_stepTest(QVector<QVector<QPointF>> &points);
    void receivedPoints_cyclicTest(QVector<QVector<QPointF>> &points);

    void setDAC_real(qreal value);
    void setTimeStart();

    void startStrokeTest();
    void startMainTest();
    void startOptionalTest(quint8 testNum);
    void runningCyclicRegulatory(const CyclicTestSettings::TestParameters &p);
    void runningCyclicShutoff(const CyclicTestSettings::TestParameters &p);
    void runningCyclicCombined(const CyclicTestSettings::TestParameters &p);
    void runningCyclicTest();


    void forwardGetParameters_mainTest(MainTestSettings::TestParameters &p) { emit getParameters_mainTest(p); }
    void forwardGetParameters_responseTest(OtherTestSettings::TestParameters &p) { emit getParameters_responseTest(p); }
    void forwardGetParameters_resolutionTest(OtherTestSettings::TestParameters &p) { emit getParameters_resolutionTest(p); }


    void endTest();
    void terminateTest();

    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoInit(int state);
};

#endif // PROGRAM_H
