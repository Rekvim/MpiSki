#include "CyclicTestSolenoid.h"

CyclicTestSolenoid::CyclicTestSolenoid(QObject *parent)
    : MainTest(parent, /*end_test_after_process=*/false)
    , m_cyclicGraphTimer(nullptr)
{}

void CyclicTestSolenoid::SetParameters(quint32 holdTimeMs,
                                       const QVector<quint16> &values,
                                       const QVector<quint32> &delaysMs,
                                       quint32 numCycles)
{
    m_holdTimeMs = holdTimeMs;
    m_values = values;
    m_delaysMs = delaysMs;
    m_numCycles = numCycles;
}

void CyclicTestSolenoid::Process()
{
    qDebug() << "@CyclicSolenoid::Process():"
             << "holdTimeMs=" << m_holdTimeMs
             << "values=" << m_values
             << "delaysMs=" << m_delaysMs
             << "numCycles=" << m_numCycles;

    emit ClearGraph();
    emit ShowDots(true);
    emit SetStartTime();

    m_cyclicGraphTimer = new QTimer(this);
    connect(m_cyclicGraphTimer, &QTimer::timeout,
            this, [&]{ emit UpdateGraph(); });
    m_cyclicGraphTimer->start(500);

    QElapsedTimer totalTimer; totalTimer.start();

    qint64 lastForwardMs = 0, lastBackwardMs = 0;

    if (!m_values.isEmpty()) {
        qDebug() << "  initial hold of" << m_values.first();
        SetDACBlocked(m_values.first(), m_holdTimeMs, true, false);
        if (m_terminate) { qDebug() << "  terminated early"; emit EndTest(); return; }
    }

    for (quint32 cycle = 0; cycle < m_numCycles; ++cycle) {
        if (m_terminate) { qDebug() << "  terminated in cycle" << cycle; emit EndTest(); return; }

        if (m_values.size() >= 2) {
            QElapsedTimer sw; sw.start();
            qDebug() << "  cycle" << cycle << "forward to" << m_values[1];
            SetDACBlocked(m_values[1],
                          m_delaysMs.value(1, m_delaysMs.last()),
                          false, true);
            lastForwardMs = sw.elapsed();
            qDebug() << "    forward took" << lastForwardMs << "ms";
            if (m_terminate) { emit EndTest(); return; }
        }

        {
            QElapsedTimer sw; sw.start();
            qDebug() << "  cycle" << cycle << "backward to" << m_values.first();
            SetDACBlocked(m_values.first(),
                          m_delaysMs.value(0, m_delaysMs.last()),
                          false, true);
            lastBackwardMs = sw.elapsed();
            qDebug() << "    backward took" << lastBackwardMs << "ms";
            if (m_terminate) { emit EndTest(); return; }
        }
    }

    m_cyclicGraphTimer->stop();

    auto [minIt, maxIt] = std::minmax_element(m_values.begin(), m_values.end());
    double rangePercent = double(*maxIt - *minIt);

    double totalTimeSec = totalTimer.elapsed() / 1000.0;

    emit SolenoidResults(
        lastForwardMs   / 1000.0,
        lastBackwardMs  / 1000.0,
        m_numCycles,
        rangePercent,
        totalTimeSec
    );

    emit EndTest();
}
