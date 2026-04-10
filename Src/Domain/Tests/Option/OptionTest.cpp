#include "OptionTest.h"

OptionTest::OptionTest(QObject *parent)
    : Test(parent)
{}

void OptionTest::Process()
{
    emit started();
    if (m_task.value.empty()) {
        emit EndTest();
        return;
    }

    setDacBlocked(m_task.value.first(), 10000, true);

    if (m_terminate) { emit EndTest(); return; }

    for (const auto &value : m_task.value) {
        setDacBlocked(value, m_task.delay);

        if (m_terminate) {
            emit EndTest();
            return;
        }
    }

    emit EndTest();
}

void OptionTest::SetTask(Task task)
{
    m_task = task;
}
