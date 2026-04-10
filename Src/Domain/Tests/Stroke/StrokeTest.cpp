#include "StrokeTest.h"

void StrokeTest::Process()
{
    emit started();
    setDacBlocked(0, 10000, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    setDacBlocked(0xFFFF, 0, true, true);

    if (m_terminate) {
        emit EndTest();
        return;
    }

    setDacBlocked(0, 0, true, true);
    emit Results();

    emit EndTest();
}
