#include "CyclicTests.h"
#include <QElapsedTimer>
#include <QtGlobal>
#include <QStringList>

CyclicTests::CyclicTests(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false)
{}

void CyclicTests::SetParameters(const Parameters& params)
{
    m_params = params;
}

QString CyclicTests::seqToString(const QVector<quint16>& seq)
{
    QStringList parts;
    parts.reserve(seq.size());
    for (quint16 v : seq) parts << QString::number(v);
    return parts.join('-');
}

void CyclicTests::Process()
{
    emit SetStartTime();
    emit ClearGraph();
    m_graphTimer->start(50);

    double totalSec = 0.0;

    switch (m_params.testType) {
    case CyclicTestSettings::TestParameters::Regulatory:
        totalSec += processRegulatory();
        break;
    case CyclicTestSettings::TestParameters::Shutoff:
        totalSec += processShutoff();
        break;
    case CyclicTestSettings::TestParameters::Combined:
        totalSec += processRegulatory();
        if (!m_terminate) totalSec += processShutoff();
        break;
    }

    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    fetchPoints(pts);

    const bool shutoffOnly =
        (m_params.testType == CyclicTestSettings::TestParameters::Shutoff);

    const QVector<quint16>& seq = shutoffOnly
                                  ? m_params.offSeqValues
                                  : m_params.regSeqValues;

    const quint16 cycles = shutoffOnly
                           ? m_params.shutoff_numCycles
                           : m_params.regulatory_numCycles;

    TestResults r;
    r.sequence = seqToString(seq);
    r.cycles = cycles;
    r.totalTimeSec = totalSec;
    if (!shutoffOnly) {
        r.ranges = m_regRanges;
    }
    r.doOnCounts = m_doOnCounts;
    r.doOffCounts = m_doOffCounts;

    calcSwitchCounts(pts, r.switch3to0Count, r.switch0to3Count);

    emit Results(r);
    emit EndTest();
}

double CyclicTests::processRegulatory()
{
    const quint32 cycles  = m_params.regulatory_numCycles;
    const quint32 delayMs = m_params.regulatory_delaySec * 1000;
    const quint32 holdMs  = m_params.regulatory_holdTimeSec * 1000;
    const auto& raw = m_params.rawRegValues;

    if (raw.isEmpty() || !delayMs || !cycles)
        return 0.0;

    SetDACBlocked(0, 0, true);

    QElapsedTimer timer; timer.start();

    for (quint32 cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < raw.size() && !m_terminate; ++i) {
            SetDACBlocked(raw.at(i), holdMs);
            if (m_terminate) return timer.elapsed() / 1000.0;
            Sleep(delayMs);
            if (m_terminate) return timer.elapsed() / 1000.0;
        }
        if (!m_terminate) emit CycleCompleted(cycle + 1);
    }

    SetDACBlocked(0, 0, true);

    {
        QVector<QVector<QPointF>> pts;
        fetchPoints(pts);
        m_regRanges = calculateRanges(pts, m_params.regSeqValues);
    }

    return timer.elapsed() / 1000.0;
}

double CyclicTests::processShutoff()
{
    const quint32 cycles = m_params.shutoff_numCycles;
    const quint32 delayMs = m_params.shutoff_delaySec * 1000;
    const quint32 holdMs = m_params.shutoff_holdTimeSec * 1000;
    const auto& raw = m_params.rawOffValues;

    if (raw.isEmpty() || !delayMs || !cycles)
        return 0.0;

    QVector<bool> doMask(m_params.shutoff_DO.begin(), m_params.shutoff_DO.end());
    const int DO_COUNT = doMask.size();
    QVector<bool> currentStates(DO_COUNT, false);

    m_doOnCounts.assign(DO_COUNT, 0);
    m_doOffCounts.assign(DO_COUNT, 0);

    SetDACBlocked(0, 0, true);

    QElapsedTimer timer; timer.start();

    for (quint32 cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < raw.size() && !m_terminate; ++i) {

            if (i != 0) {
                for (int d = 0; d < DO_COUNT; ++d) {
                    if (!doMask[d]) continue;
                    currentStates[d] = !currentStates[d];
                    currentStates[d] ? ++m_doOnCounts[d] : ++m_doOffCounts[d];
                }
                emit SetMultipleDO(currentStates);
            }

            SetDACBlocked(raw.at(i), holdMs);
            if (m_terminate) return timer.elapsed() / 1000.0;
            Sleep(delayMs);
            if (m_terminate) return timer.elapsed() / 1000.0;
        }
        if (!m_terminate) emit CycleCompleted(cycle + 1);
    }

    SetDACBlocked(0, 0, true);

    return timer.elapsed() / 1000.0;
}

void CyclicTests::fetchPoints(QVector<QVector<QPointF>>& pts)
{
    pts.clear();
    emit GetPoints(pts); // blocking
}

QVector<CyclicTests::RangeRec>
CyclicTests::calculateRanges(const QVector<QVector<QPointF>>& pts,
                             const QVector<quint16>& sequence) const
{
    QVector<RangeRec> ranges;

    if (pts.size() < 4) return ranges;

    const QVector<QPointF>& line = pts.at(2);
    const QVector<QPointF>& task = pts.at(3);
    if (line.isEmpty() || task.isEmpty()) return ranges;

    qreal prevTask = task.first().y();
    bool first = true;
    bool forward = true;

    qreal maxF = -1e9, maxR = -1e9;
    int maxFCycle = -1,  maxRCycle = -1;

    for (int i = 0; i < line.size() && i < task.size(); ++i) {
        qreal currTask = task.at(i).y();
        if (!qFuzzyCompare(currTask, prevTask)) {
            if (!first) {
                RangeRec rec;
                rec.rangePercent = static_cast<quint16>(qRound(prevTask));
                rec.maxForwardValue = (maxF < 0) ? 0.0 : maxF;
                rec.maxForwardCycle = maxFCycle;
                rec.maxReverseValue = (maxR < 0) ? 0.0 : maxR;
                rec.maxReverseCycle = maxRCycle;
                ranges.push_back(rec);
            }
            first = false;
            forward = (currTask > prevTask);
            prevTask = currTask;
            maxF = maxR = -1e9;
            maxFCycle = maxRCycle = -1;
            continue;
        }

        if (first) continue;

        qreal measured = line.at(i).y();
        if (forward) {
            if (measured > maxF) { maxF = measured; maxFCycle = -1; }
        } else {
            if (measured > maxR) { maxR = measured; maxRCycle = -1; }
        }
    }

    if (!first) {
        RangeRec rec;
        rec.rangePercent = static_cast<quint16>(qRound(prevTask));
        rec.maxForwardValue = (maxF < 0) ? 0.0 : maxF;
        rec.maxForwardCycle = maxFCycle;
        rec.maxReverseValue = (maxR < 0) ? 0.0 : maxR;
        rec.maxReverseCycle = maxRCycle;
        ranges.push_back(rec);
    }

    if (ranges.size() != sequence.size()) {
        ranges.resize(sequence.size());
        for (int i = 0; i < ranges.size(); ++i)
            ranges[i].rangePercent = sequence[i];
    }

    return ranges;
}

void CyclicTests::calcSwitchCounts(const QVector<QVector<QPointF>>& pts,
                                   int& s3to0, int& s0to3) const
{
    s3to0 = s0to3 = 0;
    if (pts.size() < 2) return;

    const QVector<QPointF>& opened = pts.at(0); // 0 -> 3
    const QVector<QPointF>& closed = pts.at(1); // 3 -> 0

    s0to3 = opened.size();
    s3to0 = closed.size();
}
