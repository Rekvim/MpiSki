#include "CyclicRegulatoryAnalyzer.h"

void CyclicRegulatoryAnalyzer::start()
{
    m_result = {};

    m_step = -1;
    m_cycle = 0;
    m_prevTask = std::numeric_limits<double>::quiet_NaN();
    m_forward = true;
}

void CyclicRegulatoryAnalyzer::configure(const CyclicTestParams& params)
{
    const auto& seq = params.regulatory.sequence;

    m_ranges.clear();
    m_result.ranges.clear();

    // берём только возрастающую часть (прямой ход)
    for (int i = 0; i < seq.size(); ++i)
    {
        if (i > 0 && seq[i] < seq[i - 1])
            break;

        m_ranges.push_back(seq[i]);

        CyclicRangeResult r;
        r.rangePercent = seq[i];
        r.maxForwardPosition = std::numeric_limits<qreal>::lowest();
        r.minReversePosition = std::numeric_limits<qreal>::max();
        r.maxForwardCycle = -1;
        r.minReverseCycle = -1;

        m_result.ranges.push_back(r);
    }
}

int CyclicRegulatoryAnalyzer::findStep(double task) const
{
    for (int i = 0; i < m_ranges.size(); ++i)
    {
        if (qAbs(m_ranges[i] - task) < 0.5)
            return i;
    }
    return -1;
}

void CyclicRegulatoryAnalyzer::onSample(const Sample& s)
{
    const double task = s.taskPercent;
    if (qIsNaN(task))
        return;

    if (qIsNaN(m_prevTask)) {
        m_prevTask = task;
        m_step = findStep(task);
        return;
    }

    if (!qFuzzyCompare(task, m_prevTask))
    {
        int newStep = findStep(task);
        if (newStep < 0)
            return;

        m_forward = task > m_prevTask;

        if (newStep < m_step)
            m_cycle++;

        m_step = newStep;
        m_prevTask = task;
    }

    updateRange(s.positionPercent);
}

void CyclicRegulatoryAnalyzer::updateRange(double pos)
{
    if (m_step < 0 || m_step >= m_result.ranges.size())
        return;

    auto& r = m_result.ranges[m_step];

    if (m_forward) {
        if (pos > r.maxForwardPosition) {
            r.maxForwardPosition = pos;
            r.maxForwardCycle = m_cycle;
        }
    } else {
        if (pos < r.minReversePosition) {
            r.minReversePosition = pos;
            r.minReverseCycle = m_cycle;
        }
    }
}

void CyclicRegulatoryAnalyzer::finish()
{
}

const CyclicRegulatoryResult&
CyclicRegulatoryAnalyzer::result() const
{
    return m_result;
}