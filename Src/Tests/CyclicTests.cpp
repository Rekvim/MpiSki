#include "CyclicTests.h"
#include <QElapsedTimer>
#include <QtGlobal>
#include <QStringList>

CyclicTests::CyclicTests(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false),
    m_patternType(SelectTests::Pattern_None)
{}

void CyclicTests::SetPatternType(SelectTests::PatternType pt)
{
    m_patternType = pt;
}

void CyclicTests::SetParameters(const Parameters& params)
{
    m_params = params;
}

void CyclicTests::Process()
{
    emit ClearGraph();

    setDacBlocked(0, 10000, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    emit SetStartTime();

    m_graphTimer->start(100);

    Sleep(5000);

    if (m_terminate) {
        emit EndTest();
        return;
    }

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

    setDacBlocked(0, 0, true);

    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    fetchPoints(pts);

    const bool shutoffOnly =
        (m_params.testType == CyclicTestSettings::TestParameters::Shutoff);

    // const QVector<qreal>& seq = shutoffOnly
    //                               ? m_params.offSeqValues
    //                               : m_params.regSeqValues;

    const quint16 cycles = shutoffOnly
                           ? m_params.shutoff_numCycles
                           : m_params.regulatory_numCycles;

    TestResults r;
    // r.sequence = seqToString(seq);
    r.cycles = cycles;
    r.totalTimeSec = totalSec;
    //r.ranges = calculateRanges(pts, seq);
    r.doOnCounts = m_doOnCounts;
    r.doOffCounts = m_doOffCounts;

    calcSwitchCounts(pts, r.switch3to0Count, r.switch0to3Count);

    emit Results(r);
    emit EndTest();
}

 double CyclicTests::processRegulatory()
 {
//     const quint16 cycles = m_params.regulatory_numCycles;
//     const quint32 delayMs = m_params.regulatory_delayMs;
//     const quint16 holdMs = m_params.regulatory_holdMs;
//     const auto& raw = m_params.rawRegValues;

//     if (raw.isEmpty() || !delayMs || !cycles)
//         return 0.0;

//     QElapsedTimer timer; timer.start();

//     for (quint16 cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
//         for (int i = 0; i < raw.size() && !m_terminate; ++i) {
//             setDacBlocked(raw.at(i), delayMs);
//             if (m_terminate) return timer.elapsed() / 1000.0;
//             Sleep(holdMs);
//             if (m_terminate) return timer.elapsed() / 1000.0;
//         }
//         if (!m_terminate) emit CycleCompleted(cycle + 1);
//     }


    return  1000.0;
}

double CyclicTests::processShutoff() {
//     const quint16 cycles = m_params.shutoff_numCycles;
//     const quint32 delayMs = m_params.shutoff_delayMs;
//     const quint16 holdMs = m_params.shutoff_holdMs;
//     // const auto& raw = m_params.rawOffValues;

//     // if (raw.isEmpty() || !delayMs || !cycles)
//     //     return 0.0;

//     QVector<bool> doMask(m_params.shutoff_DO.begin(), m_params.shutoff_DO.end());
//     const int DO_COUNT = doMask.size();
//     QVector<bool> currentStates(DO_COUNT, false);

//     m_doOnCounts.assign(DO_COUNT, 0);
//     m_doOffCounts.assign(DO_COUNT, 0);

//     setDacBlocked(0, 0, true);


//     QElapsedTimer timer; timer.start();

//     for (quint16 cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
//         for (int i = 0; i < raw.size() && !m_terminate; ++i) {

//             if (i != 0) {
//                 for (int d = 0; d < DO_COUNT; ++d) {
//                     if (!doMask[d]) continue;
//                     currentStates[d] = !currentStates[d];
//                     currentStates[d] ? ++m_doOnCounts[d] : ++m_doOffCounts[d];
//                 }
//                 emit SetMultipleDO(currentStates);
//             }

//             setDacBlocked(raw.at(i), holdMs);
//             if (m_terminate) return timer.elapsed() / 1000.0;
//             Sleep(delayMs);
//             if (m_terminate) return timer.elapsed() / 1000.0;
//         }
//         if (!m_terminate) emit CycleCompleted(cycle + 1);
//     }

//     setDacBlocked(0, 0, true);

    return 1000.0;
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

    bool useStandard =
        (m_patternType == SelectTests::Pattern_C_SOVT  ||
         m_patternType == SelectTests::Pattern_B_SACVT ||
         m_patternType == SelectTests::Pattern_C_SACVT);

    const int needed = useStandard ? 4 : 2;
    if (pts.size() < needed) return ranges;

    int baseIndex = useStandard ? 2 : pts.size() - 2;
    const auto& line = pts[baseIndex];
    const auto& task = pts[baseIndex + 1];

    if (line.isEmpty() || task.isEmpty()) return ranges;

    qreal prevLevel = task.first().y();
    bool firstSegment = true;
    bool forward = true;

    int cycleCount = 0;

    RangeRec rec;
    rec.rangePercent    = static_cast<quint16>(qRound(prevLevel));
    rec.maxForwardValue = std::numeric_limits<qreal>::lowest();
    rec.maxForwardCycle = -1;
    rec.maxReverseValue = std::numeric_limits<qreal>::max();
    rec.maxReverseCycle = -1;

    const int total = qMin(line.size(), task.size());
    for (int i = 0; i < total; ++i) {
        qreal currLevel = task[i].y();
        qreal meas      = line[i].y();

        if (!qFuzzyCompare(currLevel, prevLevel)) {
            if (!firstSegment) {
                ranges.push_back(rec);
            }
            firstSegment = false;

            forward = currLevel > prevLevel;

            if (qRound(currLevel) == sequence[0]) {
                ++cycleCount;
            }

            prevLevel = currLevel;
            rec.rangePercent     = static_cast<quint16>(qRound(currLevel));
            rec.maxForwardValue  = std::numeric_limits<qreal>::lowest();
            rec.maxForwardCycle  = -1;
            rec.maxReverseValue  = std::numeric_limits<qreal>::max();
            rec.maxReverseCycle  = -1;

            continue;
        }

        if (firstSegment) {
            continue;
        }

        if (forward) {
            if (meas > rec.maxForwardValue) {
                rec.maxForwardValue = meas;
                rec.maxForwardCycle = cycleCount;
            }
        } else {
            if (meas < rec.maxReverseValue) {
                rec.maxReverseValue = meas;
                rec.maxReverseCycle = cycleCount;
            }
        }
    }

    if (!firstSegment) {
        ranges.push_back(rec);
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
