#ifndef MAINTEST_H
#define MAINTEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>

#include "Test.h"
#include "MainTestSettings.h"

class MainTest : public Test
{
    Q_OBJECT
public:
    explicit MainTest(QObject *parent = nullptr, bool endTestAfterProcess = true);
    virtual void Process() override;
    void SetParameters(MainTestSettings::TestParameters &parameters);

    struct TestResults
    {
        qreal pressureDiff;
        qreal friction;
        qreal dinErrorMean;
        qreal dinErrorMax;
        qreal lowLimit;
        qreal highLimit;
        qreal springLow;
        qreal springHigh;
    };

protected:
    MainTestSettings::TestParameters m_parameters;

private:
    const bool m_endTestAfterProcess;
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

#endif // MAINTEST_H
