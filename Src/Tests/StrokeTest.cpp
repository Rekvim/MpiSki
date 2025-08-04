#include "StrokeTest.h"

StrokeTest::StrokeTest(QObject *parent)
    : Test(parent)
{}

void StrokeTest::SetPatternType(SelectTests::PatternType pattern)
{
    m_patternType = pattern;
}

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

    if (m_patternType == SelectTests::Pattern_C_SOVT
        || m_patternType == SelectTests::Pattern_C_SACVT
        || m_patternType == SelectTests::Pattern_B_SACVT)
    {
        processShutoff();
    }
    else
    {
        processRegulatory();
    }

    emit EndTest();
}

void StrokeTest::processRegulatory()
{
    quint64 startTime = QDateTime::currentMSecsSinceEpoch();
    SetDACBlocked(0xFFFF, 0, true, true);
    if (m_terminate) return;

    quint64 forwardTime = QDateTime::currentMSecsSinceEpoch() - startTime - 2500;

    startTime = QDateTime::currentMSecsSinceEpoch();
    SetDACBlocked(0, 0, true, true);
    if (m_terminate) return;

    quint64 backwardTime = QDateTime::currentMSecsSinceEpoch() - startTime - 2500;

    emit Results(forwardTime, backwardTime);
}

void StrokeTest::processShutoff()
{
    QVector<bool> states(2, false);

    // включаем оба DO0 и DO1
    states[0] = true;
    states[1] = true;
    emit SetMultipleDO(states);

    quint64 startTime = QDateTime::currentMSecsSinceEpoch();
    SetDACBlocked(0xFFFF, 0, true, true);
    if (m_terminate) return;
    quint64 forwardTime = QDateTime::currentMSecsSinceEpoch() - startTime - 2500;

    states[0] = false;
    states[1] = false;
    emit SetMultipleDO(states);

    startTime = QDateTime::currentMSecsSinceEpoch();
    SetDACBlocked(0, 0, true, true);
    if (m_terminate) return;
    quint64 backwardTime = QDateTime::currentMSecsSinceEpoch() - startTime - 2500;

    emit Results(forwardTime, backwardTime);
}
