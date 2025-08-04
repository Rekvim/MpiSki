    #ifndef PROGRAM_H
#define PROGRAM_H

#pragma once
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "./Src/MPI/MPI.h"
#include "OtherTestSettings.h"
#include "StepTestSettings.h"
#include "CyclicTestSettings.h"

#include "Registry.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/MainTest.h"
#include "./Src/Tests/CyclicTests.h"
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
    explicit Program(Registry& registry, QObject* parent = nullptr);
    bool isInitialized() const;

    Registry& registry() noexcept { return m_registry; }
    const TelemetryStore& telemetry() const noexcept { return m_telemetryStore; }

signals:

    void errorOccured(const QString&);

    void TelemetryUpdated(const TelemetryStore &store);
    void CyclicCycleCompleted(int completedCycles);
    void SetText(const TextObjects object, const QString &text);
    void SetTextColor(const TextObjects object, const QColor color);
    void SetTask(qreal task);
    void SetSensorNumber(quint8 num);
    void SetDOControlsEnabled(bool enable);
    void SetButtonInitEnabled(bool enable);
    void SetGroupDOVisible(bool visible);
    void SetVisible(Charts chart, quint16 series, bool visible);
    void SetRegressionEnable(bool enable);

    void getPoints(QVector<QVector<QPointF>> &points, Charts chart);

    void getPoints_mainTest(QVector<QVector<QPointF>> &points, Charts chart);
    void getPoints_optionTest(QVector<QVector<QPointF>> &points, Charts chart);
    void getPoints_cyclicTest(QVector<QVector<QPointF>> &points, Charts chart);

    void AddPoints(Charts chart, QVector<Point> points);
    void ClearPoints(Charts chart);

    void stopTheTest();
    void ShowDots(bool visible);
    void EnableSetTask(bool enable);
    void DublSeries();
    void ReleaseBlock();

    void MainTestFinished();
    void getParameters_mainTest(MainTestSettings::TestParameters &parameters);
    void getParameters_stepTest(StepTestSettings::TestParameters &parameters);
    void getParameters_resolutionTest(OtherTestSettings::TestParameters &parameters);
    void getParameters_responseTest(OtherTestSettings::TestParameters &parameters);
    void getParameters_cyclicTest(CyclicTestSettings::TestParameters &parameters);

    void Question(QString title, QString text, bool &result);
    void SetStepResults(QVector<StepTest::TestResult> results, quint32 T_value);
    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

private:
    Registry& m_registry;
    TelemetryStore m_telemetryStore;

    MPI m_mpi;
    bool m_cyclicRunning = false;
    QTimer* m_diPollTimer = nullptr;
    quint8 m_lastDI = 0;
    quint64 m_cyclicStartTs = 0;
    QTimer m_cyclicGraphTimer;
    QTimer *m_timerSensors;
    QTimer *m_timerDI;
    quint64 m_startTime;
    quint64 m_initTime;
    bool m_testing = false;
    QEventLoop *m_dacEventloop;
    bool m_stopSetDac;
    bool m_waitForButton = false;
    QVector<bool> m_initDOStates;
    QVector<bool> m_savedInitDOStates;
    std::array<bool, 4> m_enabledDO {false, false, false, false};
    bool m_isInitialized = false;
    SelectTests::PatternType m_patternType;

    qreal currentPercent() const;
    qreal calculatingK();

    // init
    void connectAndInitDevice();
    void detectAndReportSensors();
    void waitForDacCycle();
    void measureStartPosition(bool normalClosed);
    void measureStartPositionShutoff(bool normalClosed);

    void measureEndPosition(bool normalClosed);
    void measureEndPositionShutoff(bool normalClosed);

    void calculateAndApplyCoefficients();
    void recordStrokeRange(bool normalClosed);
    void finalizeInitialization();
    QVector<quint16> makeRawValues(const QVector<quint16> &seq, bool normalOpen);
private slots:
    void updateSensors();

    void SetDAC(quint16 dac,
                quint32 sleep_ms = 0,
                bool waitForStop = false,
                bool waitForStart = false);
    void SetMultipleDO(const QVector<bool>& states);

    void updateCharts_mainTest();
    void updateCharts_strokeTest();
    void updateCharts_optionTest(Charts chart);
    void updateCharts_CyclicTest();

    void results_mainTest(MainTest::TestResults results);
    void results_strokeTest(const quint64 forwardTime, const quint64 backwardTime);
    void results_stepTest(QVector<StepTest::TestResult> results, quint32 T_value);
    void results_cyclicTests(const CyclicTests::TestResults& r);

    void SetTimeStart();

public slots:
    void initialization();

    void SetInitDOStates(const QVector<bool> &states);
    void setPattern(SelectTests::PatternType pattern) { m_patternType = pattern; }

    void addRegression(const QVector<QPointF> &points);
    void addFriction(const QVector<QPointF> &points);

    void receivedPoints_mainTest(QVector<QVector<QPointF>> &points);
    void receivedPoints_stepTest(QVector<QVector<QPointF>> &points);
    void receivedPoints_cyclicTest(QVector<QVector<QPointF>> &points);

    void SetDAC_real(qreal value);

    void runningMainTest();
    void runningStrokeTest();
    void runningOptionalTest(quint8 testNum);
    void runningCyclicTest(const CyclicTestSettings::TestParameters &p);

    void endTest();
    void terminateTest();


    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoInit(int state);
};

#endif // PROGRAM_H
