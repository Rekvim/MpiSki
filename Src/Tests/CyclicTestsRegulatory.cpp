#include "CyclicTestsRegulatory.h"
#include <QThread>
CyclicTestsRegulatory::CyclicTestsRegulatory(QObject* parent, bool endTestAfterProcess)
    : MainTest(parent),
    m_endTestAfterProcess(endTestAfterProcess)
{}

void CyclicTestsRegulatory::SetPatternType(SelectTests::PatternType pt)
{
    m_patternType = pt;
}

void CyclicTestsRegulatory::Process()
{
    if (m_task.values.isEmpty()) {
        emit errorOccured("values.isEmpty() → EndTest");
        emit EndTest();
        return;
    }

    if (m_terminate) { emit EndTest(); return; }

    emit SetStartTime();
    m_graphTimer->start(100);

    quint32 cycle = 0;
    const int seqSize = m_task.sequence.size();

    for (quint32 step = 0; step < m_task.values.size() && !m_terminate; ++step) {
        const quint16 value = m_task.values.at(step);
        const qint16 rangePercent = m_task.sequence.at(step % seqSize);

        if (m_terminate) { emit EndTest(); return; }


        SetDACBlocked(value, m_task.delayMsecs);
        if (m_terminate) { emit EndTest(); return; }

        SetDACBlocked(value, m_task.holdMsecs);
        if (m_terminate) { emit EndTest(); return; }

        emit StepMeasured(rangePercent, 0.0, cycle);

        if ((step + 1) % seqSize == 0) {
            ++cycle;
            emit CycleCompleted(cycle);
        }
    }

    SetDACBlocked(0, 0, true, false);
    m_graphTimer->stop();

    QVector<QVector<QPointF>> pts;
    emit GetPoints(pts);

    TestResults r;

    r.strSequence = seqToString(m_task.sequence);
    r.ranges = calculateRanges(pts, m_task.sequence);
    emit Results(r);

    emit EndTest();
}

void CyclicTestsRegulatory::SetTask(Task task)
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
    int baseIndex = useStandard ? 2 : 0;
    const auto& line = pts[baseIndex];
    const auto& task = pts[baseIndex + 1];

    const bool hasSecond = sequence.size() > 1;
    const int seq0 = sequence[0];
    const int seq1 = hasSecond ? sequence[1] : sequence[0];

    if (line.isEmpty() || task.isEmpty()) return ranges;

    qreal prevLevel = task.first().y();
    bool firstSegment = true;
    bool forward = true;
    int cycleCount = 0;

    RangeRec rec;
    rec.rangePercent = qRound(prevLevel);
    rec.maxForwardValue = std::numeric_limits<qreal>::lowest();
    rec.maxForwardCycle = -1;
    rec.maxReverseValue = std::numeric_limits<qreal>::max();
    rec.maxReverseCycle = -1;

    const int total = qMin(line.size(), task.size());
    for (int i = 0; i < total; ++i) {
        qreal currLevel = task[i].y();
        qreal meas = line[i].y();

        if (!qFuzzyCompare(currLevel, prevLevel)) {
            if (!firstSegment &&
                (rec.maxForwardCycle >= 0 || rec.maxReverseCycle >= 0))
            {
                ranges.push_back(rec);
            }

            if (qRound(prevLevel) == seq0 && qRound(currLevel) == seq1) {
                ++cycleCount;
            }

            firstSegment = false;
            forward = currLevel > prevLevel;

            prevLevel = currLevel;
            rec.rangePercent = qRound(currLevel);
            rec.maxForwardValue = std::numeric_limits<qreal>::lowest();
            rec.maxForwardCycle = -1;
            rec.maxReverseValue = std::numeric_limits<qreal>::max();
            rec.maxReverseCycle = -1;
            continue;
        }

        if (firstSegment) continue;

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

    // «Закрываем» последний сегмент так же с фильтром
    if (!firstSegment &&
        (rec.maxForwardCycle >= 0 || rec.maxReverseCycle >= 0))
    {
        ranges.push_back(rec);
    }

    return ranges;
}
