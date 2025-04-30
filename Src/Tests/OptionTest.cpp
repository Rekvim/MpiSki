#include "OptionTest.h"

OptionTest::OptionTest(QObject *parent, bool end_test_after_process)
    : Test(parent)
    , end_test_after_process_(end_test_after_process)
{}

void OptionTest::Process()
{
    if (task_.value.empty()) {
        emit EndTest();
        return;
    }

    SetDACBlocked(task_.value.first(), 10000, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    emit SetStartTime();

    m_graphTimer->start(50);

    for (const auto value : task_.value) {
        SetDACBlocked(value, task_.delay);

        if (m_terminate) {
            emit EndTest();
            return;
        }
    }
    if (end_test_after_process_) {
        emit EndTest();
    }
}

void OptionTest::SetTask(Task task)
{
    task_ = task;
}
