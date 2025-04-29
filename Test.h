#ifndef TEST_H
#define TEST_H

#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include "MainTestSettings.h"

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr);
    virtual ~Test() = default;

protected:
    QTimer *graph_timer_;
    void Sleep(quint16 msecs);
    bool terminate_;
    void SetDACBlocked(quint16 value,
                       quint32 sleep_ms = 0,
                       bool wait_for_stop = false,
                       bool wait_for_start = false);
    QEventLoop *event_loop_;
public slots:
    virtual void Process() = 0;
    void Stop();
    void ReleaseBlock();
signals:
    void SetDAC(quint16 value,
                quint32 sleep_ms = 0,
                bool wait_for_stop = false,
                bool wait_for_start = false);
    void UpdateGraph();
    void EndTest();
};

class MainTest : public Test
{
    Q_OBJECT
public:
    explicit MainTest(QObject *parent = nullptr, bool end_test_after_process = true);
    virtual void Process() override;
    void SetParameters(MainTestSettings::TestParameters &parameters);
    struct TestResults
    {
        qreal pressure_diff;
        qreal friction;
        qreal din_error_mean;
        qreal din_error_max;
        qreal low_limit;
        qreal high_limit;
        qreal spring_low;
        qreal spring_high;
    };

protected:
    MainTestSettings::TestParameters parameters_;

private:
    const bool end_test_after_process_;
    struct Regression
    {
        qreal k;
        qreal b;
    };

    struct Limits
    {
        qreal minX;
        qreal maxX;
        qreal minY;
        qreal maxY;
    };

    Regression CalculateRegression(QVector<QPointF> &points, Limits limits);
    Limits GetLimits(const QVector<QPointF> &points1, const QVector<QPointF> &points2);
    QVector<QPointF> GetRegressionPoints(Regression regression, Limits limits);
    QVector<QPointF> GetFrictionPoints(QVector<QPointF> &points1,
                                       QVector<QPointF> &points2,
                                       Limits limits);
    QPair<qreal, qreal> GetMeanMax(QVector<QPointF> &points_forward,
                                   QVector<QPointF> &points_backward);

    QPair<qreal, qreal> GetRangeLimits(Regression regression1,
                                       Regression regression2,
                                       Limits limits);
    QPair<qreal, qreal> GetSpringLimits(Regression regression1,
                                        Regression regression2,
                                        Limits limits);
signals:
    void DublSeries();
    void GetPoints(QVector<QVector<QPointF>> &points);
    void AddRegression(const QVector<QPointF> &points);
    void AddFriction(const QVector<QPointF> &points);
    void Results(TestResults results);
    void ShowDots(bool visible);
    void ClearGraph();
};

class CyclicTest : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTest(QObject *parent = nullptr);
    void Process() override;

private:
    QTimer *cyclic_graph_timer_;
signals:
    void UpdateCyclicTred();
    void SetStartTime();
};

class StrokeTest : public Test
{
    Q_OBJECT

public:
    explicit StrokeTest(QObject *parent = nullptr);
    void Process() override;
signals:
    void SetStartTime();
    void Results(quint64 forward_time, quint64 backward_time);
};

class OptionTest : public Test
{
    Q_OBJECT

public:
    explicit OptionTest(QObject *parent = nullptr, bool end_test_after_process = true);
    virtual void Process() override;

    struct Task
    {
        QVector<quint16> value;
        quint32 delay;
    };

    void SetTask(Task task);

private:
    Task task_;
    const bool end_test_after_process_;
signals:
    void SetStartTime();
};

class StepTest : public OptionTest
{
    Q_OBJECT

public:
    explicit StepTest(QObject *parent = nullptr);
    void Process() override;

    struct TestResult
    {
        quint16 from;
        quint16 to;
        quint64 T_value;
        qreal overshoot;
    };

    void Set_T_value(quint32 T_value);

private:
    QVector<TestResult> CalculateResult(const QVector<QVector<QPointF>> &points) const;
    quint32 T_value_;
signals:
    void GetPoints(QVector<QVector<QPointF>> &points);
    void Results(QVector<TestResult> result, quint32 T_value);
};

#endif // TEST_H
