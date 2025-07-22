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
    Cyclic,
    CyclicSolenoid
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
    void SetRegistry(Registry *registry);
    bool isInitialized() const;

private:
    Registry *m_registry;

    MPI m_mpi;
    TelemetryStore m_telemetryStore;
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

    bool m_isInitialized = false;
    SelectTests::PatternType m_patternType;

    qreal currentPercent();

    void pollDIForCyclic();

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
    QVector<quint16> makeRawValues(const QString &seq, bool normalOpen);


signals:
    void TelemetryUpdated(const TelemetryStore &store);
    void CyclicCycleCompleted(int completedCycles);
    void SetText(const TextObjects object, const QString &text);
    void SetTextColor(const TextObjects object, const QColor color);
    void SetTask(qreal task);
    void SetSensorNumber(quint8 num);
    void SetButtonInitEnabled(bool enable);
    void SetGroupDOVisible(bool visible);
    void SetVisible(Charts chart, quint16 series, bool visible);
    void SetRegressionEnable(bool enable);

    void GetPoints(QVector<QVector<QPointF>> &points, Charts chart);

    void AddPoints(Charts chart, QVector<Point> points);
    void ClearPoints(Charts chart);

    void StopTest();
    void ShowDots(bool visible);
    void EnableSetTask(bool enable);
    void DublSeries();
    void ReleaseBlock();

    void MainTestFinished();
    void GetMainTestParameters(MainTestSettings::TestParameters &parameters);
    void GetStepTestParameters(StepTestSettings::TestParameters &parameters);
    void GetResolutionTestParameters(OtherTestSettings::TestParameters &parameters);
    void GetResponseTestParameters(OtherTestSettings::TestParameters &parameters);
    void GetCyclicTestParameters(CyclicTestSettings::TestParameters &parameters);

    void Question(QString title, QString text, bool &result);
    void SetStepResults(QVector<StepTest::TestResult> results, quint32 T_value);
    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

private slots:


    void SetMultipleDO(const QVector<bool>& states);
    void UpdateSensors();
    void UpdateCharts_mainTest();
    void UpdateCharts_strokeTest();
    void UpdateCharts_optionTest(Charts chart);
    // void UpdateCharts_CyclicTred();
    void UpdateCharts_CyclicSolenoid();
    void MainTestResults(MainTest::TestResults results);

    void StepTestResults(QVector<StepTest::TestResult> results, quint32 T_value);
    void SetDAC(quint16 dac,
                quint32 sleep_ms = 0,
                bool wait_for_stop = false,
                bool wait_for_start = false);
    void SetTimeStart();
    void StrokeTestResults(quint64 forward_time, quint64 backward_time);
    void SolenoidResults(QString sequence, quint16 cycles, double total_time_sec);

public slots:
    void SetInitDOStates(const QVector<bool> &states);
    void SetPattern(SelectTests::PatternType pattern) { m_patternType = pattern; }

    void onDOCounts(const QVector<int>& onCounts, const QVector<int>& offCounts);

    void AddRegression(const QVector<QPointF> &points);
    void AddFriction(const QVector<QPointF> &points);
    void GetPoints_mainTest(QVector<QVector<QPointF>> &points);
    void GetPoints_stepTest(QVector<QVector<QPointF>> &points);
    void SetDAC_real(qreal value);

    void MainTestStart();
    void onStartStrokeTest();
    void StartOptionalTest(quint8 testNum);
    void CyclicSolenoidTestStart(const CyclicTestSettings::TestParameters &p);
    void EndTest();

    void TerminateTest();

    void button_init(SelectTests::PatternType pattern);


    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoInit(int state);
};

#endif // PROGRAM_H
