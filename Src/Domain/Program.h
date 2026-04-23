#pragma once

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "Domain/Mpi/Device.h"

#include "Widgets/Chart/Manager.h"

#include "Domain/Tests/Main/Params.h"
#include "Domain/Tests/Cyclic/Params.h"
#include "Domain/Tests/Option/Step/Params.h"
#include "Domain/Tests/Option/Params.h"

#include "Storage/Registry.h"
#include "Storage/Telemetry.h"

#include "Domain/Measurement/Sample.h"
#include "Domain/Measurement/TestDataBuffer.h"
#include "Domain/Tests/IAnalyzer.h"

#include "Domain/Tests/BaseRunner.h"

#include "Domain/Tests/Option/Step/Algorithm.h"
#include "Domain/Tests/Main/Algorithm.h"
#include "Domain/Tests/Cyclic/Shutoff/Algorithm.h"
#include "Gui/Setup/SelectTests.h"

enum class TextObjects
{
    LineEdit_linearSensor,
    LineEdit_linearSensorPercent,
    LineEdit_pressureSensor_1,
    LineEdit_pressureSensor_2,
    LineEdit_pressureSensor_3,
    LineEdit_feedback_4_20mA,
};

namespace Domain {
class Program : public QObject
{
    Q_OBJECT
public:
    explicit Program(QObject *parent = nullptr);

    struct Config {
        SafePosition safePosition;
        qreal driveDiameter = 0.0;
        QString valveStroke;
        CrossingLimits crossingLimits;
        StrokeMovement strokeMovement = StrokeMovement::Linear;
        double diameterPulley = 0.0;
        double driveRangeLow = 0.0;
        double driveRangeHigh = 0.0;
        double dinamicErrorRecomend = 0.0;
    };

    void setConfig(const Config& config) { m_config = config; }

    quint8 getDIStatus() { return m_device.digitalInputs(); }
    quint8 getDOStatus() { return m_device.digitalOutputs(); }

    enum class TestWorker
    {
        None,
        Stroke,
        Main,
        Response,
        Resolution,
        Step,
        CyclicRegulatory,
        CyclicShutOff
    };


signals:
    void sampleReady(const Domain::Measurement::Sample& sample);

    void telemetryUpdated(const Telemetry& telemetry);

    void errorOccured(const QString& error);

    void cyclicCycleCompleted(int completedCycles);

    void testActuallyStarted();

    void setSensorsMask(quint8 adcMask);

    void setText(const TextObjects object, const QString &text);
    void setTextColor(const TextObjects object, const QColor color);
    void setTask(qreal task);
    void setSensorNumber(quint8 num);
    void setButtonInitEnabled(bool enable);
    void setGroupDOVisible(bool visible);
    void setVisible(Widgets::Chart::ChartType chart, quint16 series, bool visible);
    void setRegressionEnable(bool enable);

    void setStepResults(const QVector<Domain::Tests::Option::Step::Result>& results, quint32 T_value);
    void setDoButtonsChecked(quint8 status);

    void setDiCheckboxesChecked(quint8 status);
    void getPoints(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);

    void getPoints_strokeTest(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);
    void getPoints_mainTest(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);
    void getPoints_stepTest(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);
    void getPoints_cyclicTest(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);

    void addPoints(Widgets::Chart::ChartType chartType, const QVector<Widgets::Chart::Point> &points);
    void clearPoints(Widgets::Chart::ChartType chartType);

    void stopTheTest();
    void setTaskControlsEnabled(bool enable);
    void duplicateMainChartsSeries();
    void releaseBlock();

    bool question(QString& title, QString& text);

    void testFinished();

    void totalTestTimeMs(quint64 totalMs);

private:
    Config m_config;
    SelectTests::PatternType m_patternType;

    // Sample
    Domain::Measurement::Sample makeSample() const;
    void updateRealtimeTexts(const Domain::Measurement::Sample& s);
    Domain::Measurement::TestDataBuffer m_testDataBuffer;

    std::unique_ptr<IAnalyzer> m_analyzer;
    TestWorker m_testWorker = TestWorker::None;
    //


    std::unique_ptr<BaseRunner> m_activeRunner;
    void onRunnerActuallyStarted();
    template<typename RunnerT>
    void startRunner(std::unique_ptr<RunnerT> r)
    {
        m_activeRunner.reset();

        connect(r.get(), &BaseRunner::requestClearChart,
                this, [this](Widgets::Chart::ChartType chartType) {
                    emit clearPoints(chartType);
                });

        connect(r.get(), &BaseRunner::testActuallyStarted,
                this, &Program::onRunnerActuallyStarted);

        connect(r.get(), &BaseRunner::requestSetDAC,
                this, &Program::setDacRaw);

        connect(this, &Program::releaseBlock,
                r.get(), &BaseRunner::releaseBlock);

        connect(r.get(), &BaseRunner::totalTestTimeMs,
                this, &Program::totalTestTimeMs);

        connect(r.get(), &BaseRunner::endTest,
                this, &Program::endTest);

        connect(this, &Program::stopTheTest,
                r.get(), &BaseRunner::stop);

        emit setButtonInitEnabled(false);
        emit setTaskControlsEnabled(false);

        setDacRaw(0, 5000, true);

        m_activeRunner = std::move(r);
        m_activeRunner->start();
    }

    void updateCrossingStatus();

    // init
    void waitForDacCycle();
    void finalizeInitialization();

    template<typename Runner, typename... Args>
    void runTest(Args&&... args);
    void prepareShutoffTelemetry(const Tests::Cyclic::Params& params);
    void prepareRegulatoryTelemetry(const Tests::Cyclic::Params& params);

    QVector<quint16> makeRawValues(const QVector<quint16> &seq, bool normalOpen);
    QString seqToString(const QVector<quint16> &seq);

    Registry *m_registry;

    Domain::Mpi::Device m_device;

    Telemetry m_telemetry;
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
    void setMultipleDO(const QVector<bool>& states);

    void setDacRaw(quint16 dac,
                quint32 sleep_ms = 0,
                bool waitForStop = false,
                bool waitForStart = false);

    void initialization();

    // updateCharts
    void updateChartsFromSample(const Domain::Measurement::Sample& s);
    void updateMainCharts(const Domain::Measurement::Sample& s);
    void updateCyclicChart(const Domain::Measurement::Sample& s, Widgets::Chart::ChartType chartType);
    void updateTimeChart(const Domain::Measurement::Sample& s, Widgets::Chart::ChartType chartType, qint64 time);
    //
    void results_strokeTest();
    void results_mainTest(const Domain::Tests::Main::Algorithm::TestResults& results);
    void results_stepTest(const QVector<Tests::Option::Step::Result>& results, const quint32 T_value);
    void results_cyclicRegulatoryTests();
    void results_cyclicShutoffTests(const Tests::Cyclic::Shutoff::Result& results);
    void results_cyclicCombinedTests(const Tests::Cyclic::Shutoff::Result& shutoffResults);

    void setInitDoStates(const QVector<bool>& states);
    void setPattern(SelectTests::PatternType pattern) { m_patternType = pattern; }

    void addRegression(const QVector<QPointF>& points);
    void addFriction(const QVector<QPointF>& points);

    void receivedPoints_strokeTest(QVector<QVector<QPointF>>& points);
    void receivedPoints_mainTest(QVector<QVector<QPointF>>& points);
    void receivedPoints_stepTest(QVector<QVector<QPointF>>& points);
    void receivedPoints_cyclicTest(QVector<QVector<QPointF>>& points);

    void setDacReal(qreal value);

    void startStrokeTest();
    void startMainTest(const Domain::Tests::Main::Params& params);
    void startResponseTest(const Domain::Tests::Option::Params& params);
    void startResolutionTest(const Domain::Tests::Option::Params& params);
    void startStepTest(const Domain::Tests::Option::Step::Params& params);
    void startCyclicTest(const Domain::Tests::Cyclic::Params& params);
    void runCombinedCyclicTest(const Domain::Tests::Cyclic::Params& params);

    void endTest();
    void terminateTest();

    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoInit(int state);
};
}