#include "Algorithm.h"

namespace Domain::Tests::Cyclic::Regulatory {
    void Algorithm::run()
    {
        emit executionStarted();
        const auto& raw = m_task.values;
        const auto& seq = m_task.sequence;

        setDacBlocked(raw.front(), 10000, true);

        for (quint32 cycle = 0; cycle < m_task.cycles && !m_terminate; ++cycle) {
            for (int i = 0; i < raw.size() && !m_terminate; ++i) {
                const quint16 dacRaw = raw.at(i);

                const int pct = seq.at(i);
                const int prevPct = (i == 0 ? seq.last() : seq.at(i - 1));
                const bool forward = (pct > prevPct);

                setDacBlocked(dacRaw, m_task.holdMsecs + m_task.delayMsecs);
                if (m_terminate) break;

                emit stepMeasured(cycle, i, forward);
            }

            if (!m_terminate)
                emit cycleCompleted(cycle + 1);
        }
        setDacBlocked(0, 0, true);
        emit results();
        emit finished();
    }

    void Algorithm::setTask(Task task)
    {
        m_task = task;
    }
}