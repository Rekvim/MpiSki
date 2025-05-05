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

    quint64 startTime = QDateTime::currentMSecsSinceEpoch();

    SetDACBlocked(0xFFFF, 0, true, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    quint64 forwardTime = QDateTime::currentMSecsSinceEpoch() - startTime - 2500;

    startTime = QDateTime::currentMSecsSinceEpoch();

    SetDACBlocked(0, 0, true, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    quint64 backwardTime = QDateTime::currentMSecsSinceEpoch() - startTime - 2500;

    emit Results(forwardTime, backwardTime);

    emit EndTest();
}

