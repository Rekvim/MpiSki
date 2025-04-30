#include "StrokeTest.h"

StrokeTest::StrokeTest(QObject *parent)
    : Test(parent)
{}

void StrokeTest::Process()
{
    SetDACBlocked(0, 10000, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    emit SetStartTime();

    m_graphTimer->start(50);

    Sleep(5000);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    quint64 start_time = QDateTime::currentMSecsSinceEpoch();

    SetDACBlocked(0xFFFF, 0, true, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    quint64 forward_time = QDateTime::currentMSecsSinceEpoch() - start_time - 2500;

    start_time = QDateTime::currentMSecsSinceEpoch();

    SetDACBlocked(0, 0, true, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    quint64 backward_time = QDateTime::currentMSecsSinceEpoch() - start_time - 2500;

    emit Results(forward_time, backward_time);

    emit EndTest();
}

