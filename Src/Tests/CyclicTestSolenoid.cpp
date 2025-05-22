#include "CyclicTestSolenoid.h"
#include <algorithm>

CyclicTestSolenoid::CyclicTestSolenoid(QObject *parent)
    : MainTest(parent, /*isCyclic=*/false)
{
}

void CyclicTestSolenoid::SetParameters(const QString &sequence,
                                       int delaySec,
                                       int holdTimeSec,
                                       int numCycles)
{
    m_dacValues.clear();
    for (auto &part : sequence.split('-', Qt::SkipEmptyParts)) {
        bool ok = false;
        int v = part.trimmed().toInt(&ok);
        if (ok) m_dacValues.append(v);
    }

    m_stepDelayMs = delaySec * 1000;
    m_holdTimeMs = holdTimeSec * 1000;
    m_numCycles = numCycles;
}

void CyclicTestSolenoid::Process()
{
    if (m_dacValues.isEmpty() || m_stepDelayMs <= 0 || m_holdTimeMs < 0 || m_numCycles <= 0) {
        emit EndTest();
        return;
    }

    emit SetStartTime();
    emit ClearGraph();

    QElapsedTimer timer;
    timer.start();

    quint64 forwardDuration = 0;
    quint64 backwardDuration = 0;

    for (int cycle = 0; cycle < m_numCycles && !m_terminate; ++cycle) {
        for (int idx = 0; idx < m_dacValues.size() && !m_terminate; ++idx) {
            int pct = m_dacValues[idx];
            int oldPct = (idx == 0 ? m_dacValues.first() : m_dacValues[idx - 1]);

            quint64 t0 = timer.elapsed();
            emit TaskPoint(t0, oldPct);
            emit TaskPoint(t0, pct);

            emit SetDAC(pct);

            // Ждём начала движения
            QList<quint16> lineSensor;
            QElapsedTimer motionTimer;
            motionTimer.start();
            while (!m_terminate && motionTimer.elapsed() < 10000) {
                quint16 value = 0;
                emit RequestSensorRawValue(value);
                lineSensor.push_back(value);
                if (lineSensor.size() > 10)
                    lineSensor.pop_front();

                if (lineSensor.size() > 1 &&
                    qAbs(lineSensor.first() - lineSensor.last()) > 10) {
                    break;  // движение началось
                }

                QThread::msleep(50);
            }

            // Засекаем начало
            quint64 movementStart = timer.elapsed();

            // Ждём остановки
            lineSensor.clear();
            QElapsedTimer stopTimer;
            stopTimer.start();
            while (!m_terminate && stopTimer.elapsed() < 10000) {
                quint16 value = 0;
                emit RequestSensorRawValue(value);
                lineSensor.push_back(value);
                if (lineSensor.size() > 10)
                    lineSensor.pop_front();

                if (lineSensor.size() == 10 &&
                    qAbs(lineSensor.first() - lineSensor.last()) < 10) {
                    break;  // движение завершилось
                }

                QThread::msleep(50);
            }

            quint64 movementEnd = timer.elapsed();
            quint64 moveDuration = movementEnd - movementStart;

            if (pct > oldPct) {
                forwardDuration = moveDuration;
            } else if (pct < oldPct) {
                backwardDuration = moveDuration;
            }

            emit TaskPoint(movementEnd, pct);
            QThread::msleep(m_holdTimeMs);
            emit TaskPoint(timer.elapsed(), pct);

            emit UpdateCyclicTred();
        }
    }

    double totalTimeSec = timer.elapsed() / 1000.0;
    int minPct = *std::min_element(m_dacValues.constBegin(), m_dacValues.constEnd());
    int maxPct = *std::max_element(m_dacValues.constBegin(), m_dacValues.constEnd());
    double rangePercent = maxPct - minPct;

    emit SetSolenoidResults(
        forwardDuration / 1000.0,
        backwardDuration / 1000.0,
        m_numCycles,
        rangePercent,
        totalTimeSec
        );

    emit EndTest();
}

