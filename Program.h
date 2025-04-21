#ifndef PROGRAM_H
#define PROGRAM_H

#include <QColor>
#include <QObject>
#include <QPointF>

#include "Mpi.h"
#include "OtherTestSettings.h"
#include "StepTestSettings.h"
#include "CyclicTestSettings.h"

#include "Registry.h"
#include "Test.h"
#include "SelectTests.h"

enum class TextObjects {
    LineEdit_linear_sensor,
    LineEdit_linear_sensor_percent,
    LineEdit_pressure_sensor1,
    LineEdit_pressure_sensor2,
    LineEdit_pressure_sensor3,
    LineEdit_feedback_4_20mA,
    Label_status,
    Label_init,
    Label_sensors,
    Label_start_value,
    Label_end_value,
    Label_L_limit,
    Label_H_limit,

    Label_pressure_diff,
    Label_friction,
    Label_friction_percent,

    Label_din_error_mean,
    Label_din_error_mean_percent,

    Label_din_error_max,
    Label_din_error_max_percent,
    Label_range,
    Label_low_limit,
    Label_high_limit,

    Label_forward,
    Label_backward,

    LineEdit_dinamic_error,
    LineEdit_stroke,
    LineEdit_range,
    LineEdit_friction,
    LineEdit_friction_percent,
    LineEdit_forward,
    LineEdit_backward,
    LineEdit_range_pressure

};

enum class Charts {
    Main_task,
    Main_pressure,
    Main_friction,
    Response,
    Resolution,
    Stroke,
    Step,
    Trend,
    Cyclic,
    Cyclic_solenoid
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
    Registry *registry_;
    MPI mpi_;
    QTimer *timer_sensors_;
    QTimer *timer_DI_;
    quint64 start_time_;
    quint64 init_time_;
    bool testing_;
    QEventLoop *dac_eventloop_;
    bool stop_set_dac_;
    bool wait_for_button_ = false;
    SelectTests::BlockCTS block_cts_;

signals:
    void SetText(const TextObjects object, const QString &text);
    void SetTextColor(const TextObjects object, const QColor color);
    void SetTask(qreal task);
    void SetSensorNumber(quint8 num);
    void SetButtonInitEnabled(bool enable);
    void SetGroupDOVisible(bool visible);
    void SetVisible(Charts chart, quint16 series, bool visible);
    void SetRegressionEnable(bool enable);
    void SetSolenoidResults(qint64 forward, qint64 backward, quint16 cycles, double range_percent, double total_time_sec);

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
    void GetCyclicTestParameters(OtherTestSettings::TestParameters &parameters);

    void Question(QString title, QString text, bool &result);
    void SetStepResults(QVector<StepTest::TestResult> results, quint32 T_value);
    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

private slots:
    void UpdateSensors();
    void UpdateCharts_maintest();
    void UpdateCharts_stroketest();
    void UpdateCharts_optiontest(Charts chart);
    void UpdateCharts_CyclicTred();
    void MainTestResults(MainTest::TestResults results);
    void StepTestResults(QVector<StepTest::TestResult> results, quint32 T_value);
    void SetDAC(quint16 dac,
                quint32 sleep_ms = 0,
                bool wait_for_stop = false,
                bool wait_for_start = false);
    void SetTimeStart();
    void StrokeTestResults(quint64 forward_time, quint64 backward_time);
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
    void OptionalTestStart(quint8 test_num);
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
