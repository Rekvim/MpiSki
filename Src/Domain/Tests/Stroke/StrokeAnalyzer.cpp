#include "StrokeAnalyzer.h"

namespace Domain::Tests::Stroke {
void Analyzer::setConfig(const Config& cfg)
{
    m_cfg = cfg;
}

void Analyzer::start()
{
    m_samples.clear();
    m_result = {};
}

void Analyzer::onSample(const Domain::Measurement::Sample& s)
{
    m_samples.push_back(s);
}

// Медианный фильтр положения.
QVector<double> Analyzer::medianFilter(
    const QVector<Domain::Measurement::Sample>& samples,
    int window) const
{
    QVector<double> filtered;
    filtered.resize(samples.size());

    QVector<double> buf;

    for (int i = 0; i < samples.size(); ++i)
    {
        buf.clear();

        for (int j = -window/2; j <= window/2; ++j)
        {
            int idx = i + j;

            if (idx < 0 || idx >= samples.size())
                continue;

            buf.push_back(samples[idx].positionPercent);
        }

        std::sort(buf.begin(), buf.end());

        if (buf.isEmpty())
        {
            filtered[i] = samples[i].positionPercent;
            continue;
        }

        filtered[i] = buf[buf.size()/2];
    }

    return filtered;
}

// Поиск устойчивого достижения уровня.
int Analyzer::findFirstReachLevelStable(
    const QVector<double>& pos,
    int startIdx,
    double level,
    bool reachAtOrBelow,
    int confirm) const
{
    int count = 0;

    for (int i = startIdx; i < pos.size(); ++i) {
        bool ok =
            reachAtOrBelow
                ? (pos[i] <= level)
                : (pos[i] >= level);

        if (ok) {
            ++count;

            if (count >= confirm)
                return i - confirm + 1;
        } else {
            count = 0;
        }
    }

    return -1;
}


// Интерполяция времени пересечения уровня между двумя точками.
quint64 Analyzer::interpolateTime(
    const QVector<Domain::Measurement::Sample>& samples,
    const QVector<double>& pos,
    int idx,
    double level) const
{
    if (samples.isEmpty())
        return 0;

    if (idx <= 0)
        return samples[idx].testTime;

    double y1 = pos[idx - 1];
    double y2 = pos[idx];

    quint64 t1 = samples[idx - 1].testTime;
    quint64 t2 = samples[idx].testTime;

    if (qFuzzyCompare(y1, y2))
        return t2;

    double k = (level - y1) / (y2 - y1);

    return t1 + (t2 - t1) * k;
}


// Поиск момента достижения заданного уровня управляющего сигнала.
int Analyzer::findFirstTaskLevel(
    const QVector<Domain::Measurement::Sample>& samples,
    int startIdx,
    double level,
    bool reachAtOrAbove) const
{
    if (startIdx < 0 || startIdx >= samples.size())
        return -1;

    for (int i = startIdx; i < samples.size(); ++i)
    {
        const double y = samples[i].taskPercent;

        if (reachAtOrAbove ? (y >= level) : (y <= level))
            return i;
    }

    return -1;
}


// Последняя точка на уровне перед началом выхода из него.
int Analyzer::findLastAtLevelBeforeLeaving(
    const QVector<double>& pos,
    int startIdx,
    double level,
    bool atOrBelow) const
{
    if (startIdx < 0 || startIdx >= pos.size())
        return -1;

    int last = -1;

    for (int i = startIdx; i < pos.size(); ++i)
    {
        double y = pos[i];

        bool atLevel = atOrBelow ? (y <= level) : (y >= level);
        bool leaving = atOrBelow ? (y > level) : (y < level);

        if (atLevel) {
            last = i;
            continue;
        }

        if (last != -1 && leaving)
            return last;
    }

    return last;
}

// Расчёт пороговых уровней движения.
Analyzer::Thresholds
Analyzer::computeThresholds(const QVector<double>& pos) const
{
    Thresholds t;

    double minY = pos.front();
    double maxY = pos.front();

    for (double v : pos) {
        minY = std::min(minY, v);
        maxY = std::max(maxY, v);
    }

    double lowThr = minY + (maxY - minY) * 0.005;
    double highThr = maxY - (maxY - minY) * 0.005;

    bool normalClosed = m_cfg.normalClosed;

    t.forwardStart = normalClosed ? lowThr : highThr;
    t.forwardEnd = normalClosed ? highThr : lowThr;

    t.backwardStart = t.forwardEnd;
    t.backwardEnd = t.forwardStart;

    t.forwardStartLow = normalClosed;
    t.forwardEndLow = !normalClosed;

    t.backwardStartLow = t.forwardEndLow;
    t.backwardEndLow = t.forwardStartLow;

    t.forwardTask = normalClosed ? 95.0 : 5.0;
    t.backwardTask = normalClosed ? 5.0 : 95.0;

    t.forwardTaskAbove = normalClosed;
    t.backwardTaskAbove = !normalClosed;

    return t;
}

// Поиск ключевых событий движения.
Analyzer::Events
Analyzer::detectEvents(
    const QVector<Domain::Measurement::Sample>& s,
    const QVector<double>& pos,
    const Thresholds& t) const
{
    Events e;

    e.cmdForward = findFirstTaskLevel(
        s, 0, t.forwardTask, t.forwardTaskAbove);

    if (e.cmdForward < 0)
        return e;

    e.forwardStart = findLastAtLevelBeforeLeaving(
        pos, e.cmdForward, t.forwardStart, t.forwardStartLow);

    if (e.forwardStart < 0)
        return e;

    e.forwardEnd = findFirstReachLevelStable(
        pos, e.forwardStart, t.forwardEnd, t.forwardEndLow);

    if (e.forwardEnd < 0)
        return e;

    e.cmdBackward = findFirstTaskLevel(
        s, e.forwardEnd, t.backwardTask, t.backwardTaskAbove);

    if (e.cmdBackward < 0)
        return e;

    e.backwardStart = findLastAtLevelBeforeLeaving(
        pos, e.cmdBackward, t.backwardStart, t.backwardStartLow);

    if (e.backwardStart < 0)
        return e;

    e.backwardEnd = findFirstReachLevelStable(
        pos, e.backwardStart, t.backwardEnd, t.backwardEndLow);

    return e;
}

// Расчёт времени прямого и обратного хода.
Result Analyzer::computeTimes(
    const QVector<Domain::Measurement::Sample>& s,
    const QVector<double>& pos,
    const Thresholds& t,
    const Events& e) const
{
    Result r;

    if (e.forwardStart < 0 || e.forwardEnd < 0)
        return r;

    if (e.backwardStart < 0 || e.backwardEnd < 0)
        return r;

    quint64 tStartF = interpolateTime(s, pos, e.forwardStart, t.forwardStart);
    quint64 tEndF = interpolateTime(s, pos, e.forwardEnd, t.forwardEnd);

    quint64 tStartB = interpolateTime(s, pos, e.backwardStart, t.backwardStart);
    quint64 tEndB = interpolateTime(s, pos, e.backwardEnd, t.backwardEnd);

    r.forwardTimeMs = tEndF - tStartF;
    r.backwardTimeMs = tEndB - tStartB;

    return r;
}

void Analyzer::finish()
{
    if (m_samples.isEmpty())
        return;

    const QVector<Domain::Measurement::Sample>& s = m_samples;

    auto pos = medianFilter(s);

    auto thr = computeThresholds(pos);

    auto events = detectEvents(s, pos, thr);

    m_result = computeTimes(s, pos, thr, events);
}

const Result& Analyzer::result() const
{
    return m_result;
}
}