#include "StrokeTestAnalyzer.h"

void StrokeTestAnalyzer::setConfig(const Config& cfg)
{
    m_cfg = cfg;
}

void StrokeTestAnalyzer::start()
{
    m_samples.clear();
}

void StrokeTestAnalyzer::onSample(const Sample& s)
{
    m_samples.push_back(s);
}

static int findFirstTaskLevel(
    const QVector<Sample>& samples,
    int startIdx,
    double level,
    bool reachAtOrAbove)
{
    for (int i = startIdx; i < samples.size(); ++i)
    {
        double y = samples[i].taskPercent;

        if (reachAtOrAbove)
        {
            if (y >= level)
                return i;
        }
        else
        {
            if (y <= level)
                return i;
        }
    }

    return -1;
}

static int findLastAtLevelBeforeLeaving(
    const QVector<Sample>& samples,
    int startIdx,
    double level,
    bool atOrBelow)
{
    int last = -1;

    for (int i = startIdx; i < samples.size(); ++i)
    {
        double y = samples[i].positionPercent;

        bool atLevel = atOrBelow ? (y <= level) : (y >= level);
        bool leaving = atOrBelow ? (y > level) : (y < level);

        if (atLevel)
        {
            last = i;
            continue;
        }

        if (last != -1 && leaving)
            return last;
    }

    return last;
}

static int findFirstReachLevel(
    const QVector<Sample>& samples,
    int startIdx,
    double level,
    bool reachAtOrBelow)
{
    for (int i = startIdx; i < samples.size(); ++i)
    {
        double y = samples[i].positionPercent;

        if (reachAtOrBelow)
        {
            if (y <= level)
                return i;
        }
        else
        {
            if (y >= level)
                return i;
        }
    }

    return -1;
}

static quint64 dtMs(
    const QVector<Sample>& samples,
    int a,
    int b)
{
    if (a < 0 || b < 0 || b <= a)
        return 0;

    return samples[b].testTime - samples[a].testTime;
}

#include <QDebug>

static void debugPoint(
    const QVector<Sample>& samples,
    int idx,
    const QString& name)
{
    if (idx < 0 || idx >= samples.size()) {
        qDebug() << name << "NOT FOUND";
        return;
    }

    const Sample& s = samples[idx];

    qDebug()
        << name
        << "idx =" << idx
        << "time =" << s.testTime
        << "task =" << s.taskPercent
        << "pos =" << s.positionPercent;
}

StrokeTestResult StrokeTestAnalyzer::finish()
{
    StrokeTestResult r;
    qDebug() << "Samples count =" << m_samples.size();

    if (m_samples.isEmpty())
        return r;


    const QVector<Sample>& s = m_samples;

    double minY = s.front().positionPercent;
    double maxY = s.front().positionPercent;

    for (const auto& v : s)
    {
        minY = std::min(minY, double(v.positionPercent));
        maxY = std::max(maxY, double(v.positionPercent));
    }

    double lowThr = minY + (maxY - minY) * 0.005;
    double highThr = maxY - (maxY - minY) * 0.005;

    bool normalClosed = m_cfg.normalClosed;

    double forwardStartLevel = normalClosed ? lowThr : highThr;
    bool forwardStartIsLow = normalClosed;

    double forwardEndLevel = normalClosed ? highThr : lowThr;
    bool forwardEndIsLow = !normalClosed;

    double backwardStartLevel = forwardEndLevel;
    bool backwardStartIsLow = forwardEndIsLow;

    double backwardEndLevel = forwardStartLevel;
    bool backwardEndIsLow = forwardStartIsLow;

    double forwardTaskLevel = normalClosed ? 95.0 : 5.0;
    bool forwardTaskAbove = normalClosed;

    double backwardTaskLevel = normalClosed ? 5.0 : 95.0;
    bool backwardTaskAbove = !normalClosed;

    int cmdForward = findFirstTaskLevel(
        s,
        0,
        forwardTaskLevel,
        forwardTaskAbove);

    int forwardStart = findLastAtLevelBeforeLeaving(
        s,
        cmdForward,
        forwardStartLevel,
        forwardStartIsLow);

    int forwardEnd = findFirstReachLevel(
        s,
        forwardStart,
        forwardEndLevel,
        forwardEndIsLow);

    int cmdBackward = findFirstTaskLevel(
        s,
        forwardEnd,
        backwardTaskLevel,
        backwardTaskAbove);

    int backwardStart = findLastAtLevelBeforeLeaving(
        s,
        cmdBackward,
        backwardStartLevel,
        backwardStartIsLow);

    int backwardEnd = findFirstReachLevel(
        s,
        backwardStart,
        backwardEndLevel,
        backwardEndIsLow);

    r.forwardTimeMs  = dtMs(s, forwardStart, forwardEnd);
    r.backwardTimeMs = dtMs(s, backwardStart, backwardEnd);

    qDebug() << "----------- StrokeTestAnalyzer DEBUG -----------";

    debugPoint(s, cmdForward, "Forward command");
    debugPoint(s, forwardStart, "Forward start");
    debugPoint(s, forwardEnd, "Forward end");

    debugPoint(s, cmdBackward, "Backward command");
    debugPoint(s, backwardStart, "Backward start");
    debugPoint(s, backwardEnd, "Backward end");

    qDebug() << "Forward time ms =" << r.forwardTimeMs;
    qDebug() << "Backward time ms =" << r.backwardTimeMs;

    return r;
}
