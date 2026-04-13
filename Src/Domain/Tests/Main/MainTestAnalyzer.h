#pragma once

#include <QVector>
#include <QPair>

#include <QtMath>
#include <cmath>

#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/Tests/Main/MainTestResult.h"
#include "Src/Domain/Tests/IAnalyzer.h"

class MainTestAnalyzer : public IAnalyzer
{
public:
    struct Config
    {
        qreal driveDiameter = 0.0;
    };

    struct PPPoint
    {
        double pressure = 0.0; // X
        double position = 0.0; // Y
    };

    struct Limits
    {
        double minX = 0.0;
        double maxX = 0.0;
        double minY = 0.0;
        double maxY = 0.0;
    };

    struct Regression
    {
        double k = 0.0;
        double b = 0.0;
        bool valid = false;
    };

public:
    void setConfig(const Config& cfg);

    void start() override;
    void onSample(const Sample& s) override;
    void finish() override;

    const MainTestResult& result() const;

private:
    struct TimePoint
    {
        double task = 0.0;
        double position = 0.0;
    };

    struct MotionData
    {
        QVector<PPPoint> forward;
        QVector<PPPoint> backward;

        QVector<TimePoint> forwardDyn;
        QVector<TimePoint> backwardDyn;
    };

private:
    QVector<Sample> m_samples;
    Config m_cfg;
    MainTestResult m_result;

    MotionData buildMotionData() const;

    Limits computeLimits(const MotionData& data) const;

    Regression regression(
        const QVector<PPPoint>& points,
        const Limits& limits) const;

    double computeLinearity(
        const QVector<PPPoint>& points,
        const Regression& reg,
        const Limits& limits) const;

    double computePressureDiff(
        const Regression& regForward,
        const Regression& regBackward,
        const Limits& limits) const;

    double computeFrictionPercent(
        const Regression& regForward,
        const Limits& limits,
        double pressureDiff) const;

    QPair<double, double> computeDynamicErrorMeanMax(
        const QVector<TimePoint>& forward,
        const QVector<TimePoint>& backward) const;

    QPair<double, double> computeRangeLimits(
        const Regression& reg1,
        const Regression& reg2,
        const Limits& limits) const;

    QPair<double, double> computeSpringLimits(
        const Regression& reg1,
        const Regression& reg2,
        const Limits& limits) const;
};