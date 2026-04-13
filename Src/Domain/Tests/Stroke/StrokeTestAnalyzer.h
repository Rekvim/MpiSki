#pragma once

#include <QVector>

#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/Tests/IAnalyzer.h"

#include "StrokeTestResult.h"

class StrokeTestAnalyzer : public IAnalyzer
{
public:

    struct Config
    {
        bool normalClosed = true;
    };

    void setConfig(const Config& cfg);

    void start() override;
    void onSample(const Sample& s) override;
    void finish() override;
    const StrokeTestResult& result() const;

private:
    struct Thresholds
    {
        double forwardStart;
        double forwardEnd;

        double backwardStart;
        double backwardEnd;

        double forwardTask;
        double backwardTask;

        bool forwardStartLow;
        bool forwardEndLow;

        bool backwardStartLow;
        bool backwardEndLow;

        bool forwardTaskAbove;
        bool backwardTaskAbove;
    };

    Thresholds computeThresholds(const QVector<double>& pos) const;

    struct Events
    {
        int cmdForward = -1;
        int forwardStart = -1;
        int forwardEnd = -1;

        int cmdBackward = -1;
        int backwardStart = -1;
        int backwardEnd = -1;
    };

    Events detectEvents(
        const QVector<Sample>& s,
        const QVector<double>& pos,
        const Thresholds& t) const;

    StrokeTestResult computeTimes(
        const QVector<Sample>& s,
        const QVector<double>& pos,
        const Thresholds& t,
        const Events& e) const;

    QVector<double> medianFilter(
        const QVector<Sample>& samples,
        int window = 5) const;

    int findFirstReachLevelStable(
        const QVector<double>& pos,
        int startIdx,
        double level,
        bool reachAtOrBelow,
        int confirm = 3) const;

    quint64 interpolateTime(
        const QVector<Sample>& samples,
        const QVector<double>& pos,
        int idx,
        double level) const;

    int findFirstTaskLevel(
        const QVector<Sample>& samples,
        int startIdx,
        double level,
        bool reachAtOrAbove) const;

    int findLastAtLevelBeforeLeaving(
        const QVector<double>& pos,
        int startIdx,
        double level,
        bool atOrBelow) const;

private:
    Config m_cfg;
    QVector<Sample> m_samples;
    StrokeTestResult m_result;
};