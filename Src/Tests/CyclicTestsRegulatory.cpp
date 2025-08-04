#include "CyclicTestsRegulatory.h"

CyclicTestsRegulatory::CyclicTestsRegulatory(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false),
    m_patternType(SelectTests::Pattern_None)
{}

void CyclicTestsRegulatory::SetTask(const Task& task)
{
    m_task = task;
}

QString CyclicTestsRegulatory::seqToString(const QVector<quint16>& seq)
{
    QStringList parts;
    parts.reserve(seq.size());
    for (quint16 v : seq) parts << QString::number(v);
    return parts.join('-');
}

void CyclicTestsRegulatory::Process()
{
    if (m_task.values.isEmpty()) {
        emit EndTest();
        return;
    }

    SetDACBlocked(m_task.values.first(), 0, true);

    if (m_terminate) { emit EndTest(); return; }

    emit ClearGraph();
    emit SetStartTime();
    m_graphTimer->start(50);

        if (m_terminate) { emit EndTest(); return; }

    Sleep(5000);

    quint32 cycle = 0;

    for (const auto &value : m_task.values) {
        SetDACBlocked(value, m_task.delayMs);
        if (m_terminate) { emit EndTest(); return; }

        Sleep(m_task.holdMs);
        if (m_terminate) { emit EndTest(); return; }

        ++cycle;
        emit CycleCompleted(cycle);
    }

    SetDACBlocked(0, 0, true);
    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    fetchPoints(pts);

    TestResults r;
    r.ranges = calculateRanges(pts, m_task.sequence);
    emit Results(r);

    emit EndTest();
}

void CyclicTestsRegulatory::fetchPoints(QVector<QVector<QPointF>>& pts)
{
    pts.clear();
    emit GetPoints(pts); // blocking
}

QVector<CyclicTestsRegulatory::RangeRec>
CyclicTestsRegulatory::calculateRanges(const QVector<QVector<QPointF>>& pts,
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
