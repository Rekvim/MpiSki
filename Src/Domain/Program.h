#pragma once

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "Src/Domain/Mpi/Device.h"

#include "Src/CustomChart/ChartManager.h"

#include "Src/Domain/Tests/Main/MainParams.h"
#include "Src/Domain/Tests/Cyclic/CyclicParams.h"
#include "Src/Domain/Tests/Option/Step/StepParams.h"
#include "Src/Domain/Tests/Option/OptionTestParams.h"

#include "Src/Storage/Registry.h"
#include "Src/Storage/Telemetry.h"

#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/Measurement/TestDataBuffer.h"
#include "Src/Domain/Tests/IAnalyzer.h"

#include "Src/Domain/Tests/BaseRunner.h"

#include "Src/Domain/Tests/Option/Step/StepAlgorithm.h"
#include "Src/Domain/Tests/Main/MainAlgorithm.h"
#include "Src/Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.h"
#include "Src/Ui/Setup/SelectTests.h"

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
    void setRegistry(Registry *registry);
    bool isInitialized() const;
    quint8 getDIStatus() { return m_device.digitalInputs(); }
    quint8 getDOStatus() { return m_device.digitalOutputs(); }

    const Registry* registry() const { return m_registry; }

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
    void debugAnalyzer();


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
    void setVisible(Charts chart, quint16 series, bool visible);
    void setRegressionEnable(bool enable);

    void setStepResults(const QVector<Domain::Tests::Option::Step::Result>& results, quint32 T_value);
    void setDoButtonsChecked(quint8 status);

    void setDiCheckboxesChecked(quint8 status);
    void getPoints(QVector<QVector<QPointF>>& points, Charts chart);

    void getPoints_strokeTest(QVector<QVector<QPointF>>& points, Charts chart);
    void getPoints_mainTest(QVector<QVector<QPointF>>& points, Charts chart);
    void getPoints_stepTest(QVector<QVector<QPointF>>& points, Charts chart);
    void getPoints_cyclicTest(QVector<QVector<QPointF>>& points, Charts chart);

    void addPoints(Charts chart, const QVector<Point> &points);
    void clearPoints(Charts chart);

    void stopTheTest();
    void setTaskControlsEnabled(bool enable);
    void duplicateMainChartsSeries();
    void releaseBlock();

    bool question(QString& title, QString& text);

    void testFinished();

    void totalTestTimeMs(quint64 totalMs);

private:
    // Sample
    Domain::Measurement::Sample makeSample() const;
    void updateRealtimeTexts(const Domain::Measurement::Sample& s);
    Domain::Measurement::TestDataBuffer m_testDataBuffer;

    std::unique_ptr<IAnalyzer> m_analyzer;
    TestWorker m_testWorker = TestWorker::None;
    //

    SelectTests::PatternType m_patternType;

    std::unique_ptr<BaseRunner> m_activeRunner;
    void onRunnerActuallyStarted();
    template<typename RunnerT>
    void startRunner(std::unique_ptr<RunnerT> r)
    {
        m_activeRunner.reset();

        connect(r.get(), &BaseRunner::requestClearChart,
                this, [this](Charts chart) {
                    emit clearPoints(chart);
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
    void updateCyclicChart(const Domain::Measurement::Sample& s, Charts chart);
    void updateTimeChart(const Domain::Measurement::Sample& s, Charts chart, qint64 time);
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