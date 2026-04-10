#include "CyclicRegulatoryAnalyzer.h"

void CyclicRegulatoryAnalyzer::configure(const CyclicTestParams& params)
{
    m_ranges.clear();
    m_ranges.reserve(params.regSeqValues.size());

    m_result.ranges.clear();
    m_result.ranges.resize(params.regSeqValues.size());

    for (int i = 0; i < params.regSeqValues.size(); ++i)
    {
        const int range = params.regSeqValues[i];

        m_ranges.push_back(range);

        auto& r = m_result.ranges[i];

        r.rangePercent = range;
        r.maxForwardPosition = std::numeric_limits<qreal>::lowest();
        r.minReversePosition = std::numeric_limits<qreal>::max();
    }
}

void CyclicRegulatoryAnalyzer::onSample(const Sample& s)
{
    const double task = s.taskPercent;

    if (qIsNaN(task))
        return;

    if (qIsNaN(m_prevTask)) {
        m_prevTask = task;
        return;
    }

    if (!qFuzzyCompare(task, m_prevTask))
    {
        int newStep = m_ranges.indexOf(qRound(task));

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

    if (m_forward)
    {
        if (pos > r.maxForwardPosition)
        {
            r.maxForwardPosition = pos;
            r.maxForwardCycle = m_cycle;
        }
    }
    else
    {
        if (pos < r.minReversePosition)
        {
            r.minReversePosition = pos;
            r.minReverseCycle = m_cycle;
        }
    }
}

const CyclicRegulatoryResult&
CyclicRegulatoryAnalyzer::result() const
{
    return m_result;
}

void CyclicRegulatoryAnalyzer::reset()
{
    m_result = {};
}