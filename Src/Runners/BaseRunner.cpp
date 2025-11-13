#include "./Src/Runners/BaseRunner.h"
#include "./Src/Tests/Test.h"
#include "./Src/Runners/ITestRunner.h"
#include "./Src/Mpi/Mpi.h"
#include "./Registry.h"

BaseRunner::BaseRunner(Mpi& mpi, Registry& reg, QObject* parent)
    : ITestRunner(parent), m_mpi(mpi), m_reg(reg) {}

BaseRunner::~BaseRunner() {}

void BaseRunner::start() {
    auto cfg = buildConfig();
    if (!cfg.worker) {
        emit endTest();
        return;
    }
    emit requestClearChart(cfg.chartToClear);

    m_worker = cfg.worker;

    if (cfg.chartToClear >= 0)
        emit clearPoints(cfg.chartToClear);
    if (cfg.totalMs)
        emit totalTestTimeMs(cfg.totalMs);

    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started,
            m_worker, &Test::Process);

    connect(m_worker, &Test::EndTest,
            m_thread, &QThread::quit);

    connect(m_worker, SIGNAL(SetDAC(quint16,quint32,bool,bool)),
            this, SIGNAL(requestSetDAC(quint16,quint32,bool,bool)),
            Qt::QueuedConnection);

    connect(this, SIGNAL(releaseBlock()),
            m_worker, SLOT(ReleaseBlock()),
            Qt::QueuedConnection);

    connect(m_thread, &QThread::finished,
            m_thread, &QObject::deleteLater);

    connect(m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);

    connect(m_worker, &Test::EndTest,
            this, &BaseRunner::endTest);

    wireSpecificSignals(*m_worker);

    m_thread->start();
}

void BaseRunner::stop() {
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "StoppingTheTest", Qt::QueuedConnection);
    }
}

void BaseRunner::releaseBlock() {
    if (m_worker)
        QMetaObject::invokeMethod(m_worker, "ReleaseBlock", Qt::QueuedConnection);
}
