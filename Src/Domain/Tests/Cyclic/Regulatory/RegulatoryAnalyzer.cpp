#include "RegulatoryAnalyzer.h"

namespace Domain::Tests::Cyclic::Regulatory {
    void Analyzer::start()
    {
        m_result = {};
    }

    void Analyzer::configure(const Params& params)
    {
        const auto& seq = params.sequence;

        m_ranges.clear();
        m_result.ranges.clear();

        if (seq.empty())
            return;

        // Определение направления хода: прямой или обратный
        bool isAscending = (seq.size() == 1) || (seq[1] > seq[0]);

        for (int i = 0; i < seq.size(); ++i)
        {
            if (i > 0) {
                if (isAscending && seq[i] < seq[i - 1])
                    break;
                if (!isAscending && seq[i] > seq[i - 1])
                    break;
            }

            m_ranges.push_back(seq[i]);

            Range r;

            r.rangePercent = seq[i];
            r.maxForwardPosition = std::numeric_limits<qreal>::lowest();
            r.minBackwardPosition = std::numeric_limits<qreal>::max();
            r.maxForwardCycle = -1;
            r.minBackwardCycle = -1;

            m_result.ranges.push_back(r);
        }
    }

    int Analyzer::findStep(double task) const
    {
        for (int i = 0; i < m_ranges.size(); ++i)
        {
            if (qFuzzyCompare(1.0 + m_ranges[i], 1.0 + task))
                return i;
        }
        return -1;
    }

    void Analyzer::onSample(const Domain::Measurement::Sample& s)
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

            m_isForward = task > m_prevTask;

            if (newStep < m_step)
                m_cycle++;

            m_step = newStep;
            m_prevTask = task;
        }

        updateRange(s.positionPercent);
    }

    void Analyzer::updateRange(double pos)
    {
        if (m_step < 0 || m_step >= m_result.ranges.size())
            return;

        if (m_result.ranges.empty())
            return;

        auto& r = m_result.ranges[m_step];

        if (m_isForward) {
            if (pos > r.maxForwardPosition) {
                r.maxForwardPosition = pos;
                r.maxForwardCycle = m_cycle;
            }
        } else {
            if (pos < r.minBackwardPosition) {
                r.minBackwardPosition = pos;
                r.minBackwardCycle = m_cycle;
            }
        }
    }

    void Analyzer::finish()
    {
    }

    const Result& Analyzer::result() const
    {
        return m_result;
    }
}