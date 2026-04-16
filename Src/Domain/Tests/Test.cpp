#include "Test.h"
#include <QTimer>

Test::Test(QObject *parent)
    : QObject{parent}
{
    m_eventLoop = new QEventLoop(this);
}

void Test::Sleep(quint16 msecs)
{
    QEventLoop loop(this);
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void Test::setDacBlocked(quint16 value, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    emit dacCommandRequested(value, sleepMs, waitForStop, waitForStart);
    m_eventLoop->exec();
}

void Test::requestStop()
{
    m_terminate = true;
    m_eventLoop->quit();
}

void Test::releaseWait()
{
    m_eventLoop->quit();
}
