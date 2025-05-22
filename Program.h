#ifndef PROGRAM_H
#define PROGRAM_H

#pragma once
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "Mpi.h"
#include "OtherTestSettings.h"
#include "StepTestSettings.h"
#include "CyclicTestSettings.h"

#include "Registry.h"
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
    Label_status,
    Label_init,
    Label_sensors,
    Label_startValue,
    Label_endValue,

    Label_lowLimit,
    Label_highLimit,

    Label_pressureDiff,
    Label_friction,
    Label_frictionPercent,

    Label_din_error_mean,
    Label_din_error_mean_percent,

    Label_din_error_max,
    Label_din_error_max_percent,
    Label_range,
    Label_low_limit,
    Label_high_limit,

    Label_strokeTest_forwardTime,
    Label_strokeTest_backwardTime,

    LineEdit_dinamicError,
    LineEdit_stroke,
    LineEdit_range,
    LineEdit_friction,
    LineEdit_frictionPercent,
    LineEdit_strokeTest_forwardTime,
    LineEdit_strokeTest_backwardTime,
    LineEdit_rangePressure

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
    void SetBlockCTS(const SelectTests::BlockCTS &blockCTS);

private:
    Registry *m_registry;
    MPI m_mpi;
    QTimer *m_timerSensors;
    QTimer *m_timerDI;
    quint64 m_startTime;
    quint64 m_initTime;
    bool m_testing;
    QEventLoop *m_dacEventloop;
    bool m_stopSetDac;
    bool m_waitForButton = false;
    SelectTests::BlockCTS m_blockCts;

signals:
    void SetText(const TextObjects object, const QString &text);
    void SetTextColor(const TextObjects object, const QColor color);
    void SetTask(qreal task);
    void SetSensorNumber(quint8 num);
    void SetButtonInitEnabled(bool enable);
    void SetGroupDOVisible(bool visible);
    void SetVisible(Charts chart, quint16 series, bool visible);
    void SetRegressionEnable(bool enable);
    void SetSolenoidResults(double forwardSec, double backwardSec, quint16 cycles, double rangePercent,  double totalTimeSec);

    void GetPoints(QVector<QVector<QPointF>> &points, Charts chart);

    void AddPoints(Charts chart, QVector<Point> points);
    void ClearPoints(Charts chart);

    void StopTest();
    void ShowDots(bool visible);
    void EnableSetTask(bool enable);
    void DublSeries();
    void ReleaseBlock();

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

    void UpdateSensors();
    void UpdateCharts_maintest();
    void UpdateCharts_stroketest();
    void UpdateCharts_optiontest(Charts chart);
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
    void SolenoidResults(double forward, double backward, quint16 cycles, double range_percent, double total_time_sec);

public slots:
    void AddRegression(const QVector<QPointF> &points);
    void AddFriction(const QVector<QPointF> &points);
    void GetPoints_maintest(QVector<QVector<QPointF>> &points);
    void GetPoints_steptest(QVector<QVector<QPointF>> &points);
    void EndTest();
    void SetDAC_real(qreal value);
    void SetDAC_int(quint16 value);
    void button_init();
    void MainTestStart();
    void StrokeTestStart();
    void StartOptionalTest(quint8 testNum);
    void CyclicSolenoidTestStart(const CyclicTestSettings::TestParameters &p);

    void TerminateTest();
    void button_open();
    void button_report();
    void button_pixmap1();
    void button_pixmap2();
    void button_pixmap3();
    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoinit(int state);
};

#endif // PROGRAM_H
