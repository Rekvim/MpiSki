#include "StrokeTest.h"

void StrokeTest::run()
{
    emit executionStarted();
    setDacBlocked(0, 10000, true);

    if (m_terminate) {
        emit finished();
        return;
    }

    setDacBlocked(0xFFFF, 0, true, true);

    if (m_terminate) {
        emit finished();
        return;
    }

    setDacBlocked(0, 0, true, true);
    emit results();

    emit finished();
}
