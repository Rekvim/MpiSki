#include "BaseRunner.h"
#include "Test.h"

#include <QMetaObject>

BaseRunner::~BaseRunner()
{
    stop();
    cleanupThread();
}

void BaseRunner::cleanupThread()
{
    if (!m_thread)
        return;

    m_thread->requestInterruption();
    m_thread->quit();
    m_thread->wait();

    m_thread = nullptr;
    m_worker = nullptr;
}

void BaseRunner::start()
{
    if (m_thread || m_worker)
        return;

    RunnerConfig cfg = buildConfig();
    if (!cfg.worker) {
        emit endTest();
        return;
    }

    if (cfg.totalMs > 0)
        emit totalTestTimeMs(cfg.totalMs);

    if (cfg.chartToClear != Widgets::Chart::ChartType::None)
        emit requestClearChart(cfg.chartToClear);

    QThread* thread = new QThread(this);
    Test* worker = cfg.worker.release();

    m_thread = thread;
    m_worker = worker;

    worker->moveToThread(thread);

    connect(thread, &QThread::started,
            worker, &Test::run);

    connect(worker, &Test::executionStarted,
            this, &BaseRunner::testActuallyStarted,
            Qt::QueuedConnection);

    connect(worker, &Test::dacCommandRequested,
            this, &BaseRunner::requestSetDAC,
            Qt::QueuedConnection);

    connect(worker, &Test::finished,
            thread, &QThread::quit,
            Qt::QueuedConnection);

    connect(worker, &Test::finished,
            this, &BaseRunner::endTest,
            Qt::QueuedConnection);

    connect(thread, &QThread::finished,
            worker, &QObject::deleteLater);

    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);

    connect(thread, &QThread::finished,
            this, [this, thread, worker]() {
                if (m_worker == worker) m_worker = nullptr;
                if (m_thread == thread) m_thread = nullptr;
            }, Qt::QueuedConnection);

    wireSpecificSignals(*worker);
    thread->start();
}

void BaseRunner::stop()
{
    if (!m_worker) return;
    QMetaObject::invokeMethod(m_worker, "StoppingTheTest", Qt::QueuedConnection);
}

void BaseRunner::releaseBlock()
{
    if (!m_worker) return;
    QMetaObject::invokeMethod(m_worker, "ReleaseBlock", Qt::QueuedConnection);
}