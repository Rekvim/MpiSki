#pragma once

#include "Src/Domain/Tests/Test.h"
#include "MainTestParams.h"
#include <QPointF>

class MainTest : public Test
{
    Q_OBJECT
public:
    explicit MainTest(QObject *parent = nullptr, bool endTestAfterProcess = true);
    virtual void run() override;
    void setParameters(MainTestParams &parameters);

    struct TestResults
    {
        double pressureDiff;
        double friction;
        double dynamicErrorMean;
        double dynamicErrorMax;
        double lowLimitPressure;
        double highLimitPressure;
        double springLow;
        double springHigh;

        double linearityError;
        double linearity;
    };

protected:
    MainTestParams m_params;

private:
    const bool m_endTestAfterProcess;
    struct Regression
    {
        double k;
        double b;
    };

    struct Limits
    {
        double minX;
        double maxX;
        double minY;
        double maxY;
    };

    Regression calculateRegression(const QVector<QPointF> &points, Limits limits);

    Limits limits(const QVector<QPointF> &points1, const QVector<QPointF> &points2);

    QVector<QPointF> regressionPoints(Regression regression, Limits limits);

    double linearityError(const QVector<QPointF>& points,
                                      const Regression& regression,
                                      const Limits& limits);

    QVector<QPointF> frictionPoints(const QVector<QPointF> &points1,
                                       const QVector<QPointF> &points2,
                                       Limits limits);

    QPair<double, double> meanMax(const QVector<QPointF> &points_forward,
                                   const QVector<QPointF> &points_backward);

    QPair<double, double> rangeLimits(Regression regression1,
                                       Regression regression2,
                                       Limits limits);
    QPair<double, double> springLimits(Regression regression1,
                                        Regression regression2,
                                        Limits limits);
signals:
    void requestSensorRawValue(quint16 &value);
    void dublSeries();
    void getPoints(QVector<QVector<QPointF>> &points);
    void addRegression(const QVector<QPointF> &points);
    void addFriction(const QVector<QPointF> &points);
    void results(TestResults results);
};
