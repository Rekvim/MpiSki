#include "Algorithm.h"

namespace Domain::Tests::Option {
    void Algorithm::run()
    {
        emit executionStarted();
        if (m_task.value.empty()) {
            emit finished();
            return;
        }

        setDacBlocked(m_task.value.first(), 10000, true);

        if (m_terminate) {
            emit finished();
            return;
        }

        for (const auto &value : m_task.value) {
            setDacBlocked(value, m_task.delay);

            if (m_terminate) {
                emit finished();
                return;
            }
        }

        emit finished();
    }

    void Algorithm::setTask(Task task)
    {
        m_task = task;
    }
}