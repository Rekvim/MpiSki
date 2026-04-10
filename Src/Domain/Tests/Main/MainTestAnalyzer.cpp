#include "MainTestAnalyzer.h"
#include <algorithm>

void MainTestAnalyzer::start()
{
    m_samples.clear();
}

void MainTestAnalyzer::onSample(const Sample& s)
{
    m_samples.push_back(s);
}

void MainTestAnalyzer::setConfig(const Config& cfg)
{
    m_cfg = cfg;
}

MainTestAnalyzer::Limits
MainTestAnalyzer::computeLimits(const MotionData& data) const
{
    Limits result;

    if (data.forward.isEmpty() || data.backward.isEmpty())
        return result;

    auto initByFirst = [&](const PPPoint& p) {
        result.minX = result.maxX = p.pressure;
        result.minY = result.maxY = p.position;
    };

    initByFirst(data.forward.first());

    auto update = [&](const QVector<PPPoint>& points) {
        for (const PPPoint& p : points) {
            result.minX = std::min(result.minX, p.pressure);
            result.maxX = std::max(result.maxX, p.pressure);
            result.minY = std::min(result.minY, p.position);
            result.maxY = std::max(result.maxY, p.position);
        }
    };

    update(data.forward);
    update(data.backward);

    return result;
}

MainTestAnalyzer::Regression
MainTestAnalyzer::regression(
    const QVector<PPPoint>& points,
    const Limits& limits) const
{
    Regression result;

    if (points.isEmpty())
        return result;

    const double rangeFrac = 0.10;

    const double minY =
        (limits.maxY - limits.minY) * rangeFrac + limits.minY;

    const double maxY =
        limits.maxY - (limits.maxY - limits.minY) * rangeFrac;

    double xy = 0.0;
    double x = 0.0;
    double y = 0.0;
    double x2 = 0.0;
    int n = 0;

    for (const PPPoint& p : points)
    {
        if (p.position < minY || p.position > maxY)
            continue;

        ++n;
        xy += p.pressure * p.position;
        x += p.pressure;
        y += p.position;
        x2 += p.pressure * p.pressure;
    }

    if (n < 2)
        return result;

    const double denom = n * x2 - x * x;

    if (qFuzzyIsNull(denom))
        return result;

    result.k = (n * xy - x * y) / denom;
    result.b = (y - result.k * x) / n;
    result.valid = std::isfinite(result.k) && std::isfinite(result.b);

    return result;
}

double MainTestAnalyzer::computeLinearity(
    const QVector<PPPoint>& points,
    const Regression& reg,
    const Limits& limits) const
{
    if (!reg.valid || points.isEmpty())
        return 0.0;

    const double frac = 0.10;

    const double minY =
        limits.minY + (limits.maxY - limits.minY) * frac;

    const double maxY =
        limits.maxY - (limits.maxY - limits.minY) * frac;

    const double yRange = maxY - minY;

    if (yRange <= 0.0)
        return 0.0;

    double maxDiff = 0.0;

    for (const PPPoint& p : points)
    {
        if (p.position < minY || p.position > maxY)
            continue;

        const double yLin = reg.k * p.pressure + reg.b;
        const double diff = qAbs(p.position - yLin);

        if (diff > maxDiff)
            maxDiff = diff;
    }

    return (maxDiff / yRange) * 100.0;
}

double MainTestAnalyzer::computePressureDiff(
    const Regression& regForward,
    const Regression& regBackward,
    const Limits& limits) const
{
    if (!regForward.valid || !regBackward.valid)
        return 0.0;

    if (qFuzzyIsNull(regForward.k) || qFuzzyIsNull(regBackward.k))
        return 0.0;

    const double yMean = (limits.maxY + limits.minY) / 2.0;

    const double xForward = (yMean - regForward.b) / regForward.k;
    const double xBackward = (yMean - regBackward.b) / regBackward.k;

    if (!std::isfinite(xForward) || !std::isfinite(xBackward))
        return 0.0;

    return qAbs(xForward - xBackward);
}

MainTestAnalyzer::MotionData MainTestAnalyzer::buildMotionData() const
{
    MotionData data;

    if (m_samples.size() < 3)
        return data;

    int peakIdx = 0;
    double peakDac = m_samples.first().dac;

    for (int i = 1; i < m_samples.size(); ++i) {
        if (m_samples[i].dac > peakDac) {
            peakDac = m_samples[i].dac;
            peakIdx = i;
        }
    }

    auto appendSample = [&](const Sample& s, QVector<PPPoint>& pp, QVector<TimePoint>& dyn) {
        if (qIsNaN(s.pressure1) || qIsNaN(s.positionPercent) || qIsNaN(s.taskPercent))
            return;

        PPPoint ppPoint;
        ppPoint.pressure = s.pressure1;
        ppPoint.position = s.positionPercent;
        pp.push_back(ppPoint);

        TimePoint tp;
        tp.task = s.taskPercent;
        tp.position = s.positionPercent;
        dyn.push_back(tp);
    };

    for (int i = 0; i <= peakIdx; ++i)
        appendSample(m_samples[i], data.forward, data.forwardDyn);

    for (int i = peakIdx; i < m_samples.size(); ++i)
        appendSample(m_samples[i], data.backward, data.backwardDyn);

    return data;
}

double MainTestAnalyzer::computeFrictionPercent(
    const Regression& regForward,
    const Limits& limits,
    double pressureDiff) const
{
    if (!regForward.valid || qFuzzyIsNull(regForward.k))
        return 0.0;

    const double xMin =
        (limits.minY - regForward.b) / regForward.k;

    const double xMax =
        (limits.maxY - regForward.b) / regForward.k;

    const double range = qAbs(xMin - xMax);

    if (range <= 0.0 || !std::isfinite(range))
        return 0.0;

    return 50.0 * pressureDiff / range;
}

QPair<double, double> MainTestAnalyzer::computeDynamicErrorMeanMax(
    const QVector<TimePoint>& forward,
    const QVector<TimePoint>& backward) const
{
    if (forward.isEmpty() || backward.isEmpty())
        return qMakePair(0.0, 0.0);

    double minY = forward.first().position;
    double maxY = forward.first().position;

    auto updateMinMax = [&](const QVector<TimePoint>& pts) {
        for (const TimePoint& p : pts) {
            minY = std::min(minY, p.position);
            maxY = std::max(maxY, p.position);
        }
    };

    updateMinMax(forward);
    updateMinMax(backward);

    if (maxY <= minY)
        return qMakePair(0.0, 0.0);

    const int minSize = int(std::min(forward.size(), backward.size()));
    const int sections = std::max(1, minSize / 3);

    const double step = (maxY - minY) / sections;

    if (step <= 0.0)
        return qMakePair(0.0, 0.0);

    QVector<int> numF(sections, 0);
    QVector<int> numB(sections, 0);

    QVector<double> sumF(sections, 0.0);
    QVector<double> sumB(sections, 0.0);

    auto sectionIndex = [&](double y) -> int
    {
        int idx = qFloor((y - minY) / step);
        idx = std::clamp(idx, 0, sections - 1);
        return idx;
    };

    for (const TimePoint& p : forward)
    {
        const int idx = sectionIndex(p.position);
        ++numF[idx];
        sumF[idx] += p.task;
    }

    for (const TimePoint& p : backward)
    {
        const int idx = sectionIndex(p.position);
        ++numB[idx];
        sumB[idx] += p.task;
    }

    double sum = 0.0;
    int count = 0;
    double maxDiff = 0.0;

    const int begin = int(sections * 0.05);
    const int end   = int(sections * 0.95);

    for (int i = begin; i < end; ++i)
    {
        if (numF[i] == 0 || numB[i] == 0)
            continue;

        const double meanF = sumF[i] / numF[i];
        const double meanB = sumB[i] / numB[i];

        const double diff = qAbs(meanF - meanB);

        sum += diff;
        ++count;
        maxDiff = std::max<double>(maxDiff, diff);
    }

    if (count == 0)
        return qMakePair(0.0, 0.0);

    return qMakePair(sum / count, maxDiff);
}

QPair<double, double> MainTestAnalyzer::computeSpringLimits(
    const Regression& reg1,
    const Regression& reg2,
    const Limits& limits) const
{
    if (!reg1.valid || !reg2.valid)
        return qMakePair(0.0, 0.0);

    if (qFuzzyIsNull(reg1.k) || qFuzzyIsNull(reg2.k))
        return qMakePair(0.0, 0.0);

    auto xVal = [](const Regression& reg, double y)
    {
        return (y - reg.b) / reg.k;
    };

    const double x1 = xVal(reg1, limits.minY);
    const double x2 = xVal(reg1, limits.maxY);
    const double x3 = xVal(reg2, limits.minY);
    const double x4 = xVal(reg2, limits.maxY);

    const double minX =
        (std::min(x1, x2) + std::min(x3, x4)) / 2.0;

    const double maxX =
        (std::max(x1, x2) + std::max(x3, x4)) / 2.0;

    if (!std::isfinite(minX) || !std::isfinite(maxX))
        return qMakePair(0.0, 0.0);

    return qMakePair(minX, maxX);
}

QPair<double, double> MainTestAnalyzer::computeRangeLimits(
    const Regression& reg1,
    const Regression& reg2,
    const Limits& limits) const
{
    if (!reg1.valid || !reg2.valid)
        return qMakePair(0.0, 0.0);

    if (qFuzzyIsNull(reg1.k) || qFuzzyIsNull(reg2.k))
        return qMakePair(0.0, 0.0);

    auto xVal = [](const Regression& reg, double y)
    {
        return (y - reg.b) / reg.k;
    };

    const double x1 = xVal(reg1, limits.minY);
    const double x2 = xVal(reg1, limits.maxY);
    const double x3 = xVal(reg2, limits.minY);
    const double x4 = xVal(reg2, limits.maxY);

    const double minX = std::min(std::min(x1, x2), std::min(x3, x4));
    const double maxX = std::max(std::max(x1, x2), std::max(x3, x4));

    if (!std::isfinite(minX) || !std::isfinite(maxX))
        return qMakePair(0.0, 0.0);

    return qMakePair(minX, maxX);
}

MainTestResult MainTestAnalyzer::finish()
{
    MainTestResult r;

    if (m_samples.isEmpty())
        return r;

    const MotionData data = buildMotionData();

    if (data.forward.isEmpty() || data.backward.isEmpty())
        return r;

    const Limits limits = computeLimits(data);

    const Regression regForward = regression(data.forward, limits);
    const Regression regBackward = regression(data.backward, limits);

    if (!regForward.valid || !regBackward.valid)
        return r;

    r.linearityError = std::max<double>(
        computeLinearity(data.forward, regForward, limits),
        computeLinearity(data.backward, regBackward, limits));

    r.linearity = 100.0 - r.linearityError;

    r.pressureDiff = computePressureDiff(regForward, regBackward, limits);

    const qreal forceCoef =
        5.0 * M_PI * m_cfg.driveDiameter * m_cfg.driveDiameter / 4.0;

    r.frictionForce = r.pressureDiff * forceCoef;
    r.frictionPercent =
        computeFrictionPercent(regForward, limits, r.pressureDiff);

    QPair<double, double> dyn =
        computeDynamicErrorMeanMax(data.forwardDyn, data.backwardDyn);

    r.dynamicErrorMean = dyn.first;
    r.dynamicErrorMax = dyn.second;

    QPair<double, double> range =
        computeRangeLimits(regForward, regBackward, limits);

    r.lowLimitPressure = range.first;
    r.highLimitPressure = range.second;

    QPair<double, double> spring =
        computeSpringLimits(regForward, regBackward, limits);

    r.springLow = spring.first;
    r.springHigh = spring.second;

    return r;
}