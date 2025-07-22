// CyclicTestSolenoid.cpp

#include "CyclicTestSolenoid.h"
#include <QThread>
#include <QElapsedTimer>

CyclicTestSolenoid::CyclicTestSolenoid(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false)
{ }

void CyclicTestSolenoid::SetParameters(const CyclicTestSettings::TestParameters& params)
{
    m_params = params;
    parseSequence(params.regulatory_sequence, m_valuesReg);
    parseSequence(params.shutoff_sequence, m_valuesOff);

    // raw‑значения переданы из Program::CyclicSolenoidTestStart
    m_rawValuesReg = params.rawRegValues;
    m_rawValuesOff = params.rawOffValues;
}

void CyclicTestSolenoid::parseSequence(const QString& seq, QVector<int>& out)
{
    out.clear();
    for (const QString& part : seq.split('-', Qt::SkipEmptyParts)) {
        bool ok = false;
        int v = part.trimmed().toInt(&ok);
        if (ok) out.append(v);
    }
}

void CyclicTestSolenoid::Process()
{
    emit SetStartTime();
    emit ClearGraph();

    m_graphTimer->start(50);

    switch (m_params.testType) {
    case CyclicTestSettings::TestParameters::Regulatory:
        processRegulatory();
        break;
    case CyclicTestSettings::TestParameters::Shutoff:
        processShutoff();
        break;
    case CyclicTestSettings::TestParameters::Combined:
        processRegulatory();
        if (!m_terminate)
            processShutoff();
        break;
    }

    m_graphTimer->stop();
    emit EndTest();
}

void CyclicTestSolenoid::processRegulatory()
{
    const quint32 cycles = m_params.regulatory_numCycles;
    const quint32 delayMs = m_params.regulatory_delaySec * 1000;
    const quint32 holdMs = m_params.regulatory_holdTimeSec * 1000;
    const auto& raw = m_rawValuesReg;

    if (raw.isEmpty() || delayMs == 0 || cycles == 0)
        return;

    QElapsedTimer timer; timer.start();

    for (quint32 cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < raw.size() && !m_terminate; ++i) {
            quint16 dacRaw = raw.at(i);
            int pct = m_valuesReg.at(i);
            int prevPct = (i == 0 ? m_valuesReg.last() : m_valuesReg.at(i-1));
            bool forward= (pct > prevPct);

            SetDACBlocked(dacRaw, holdMs,
                          /*wait_for_stop=*/true,
                          /*wait_for_start=*/false);
            if (m_terminate) return;

            Sleep(delayMs);
            if (m_terminate) return;

            emit RegulatoryMeasurement(cycle, i, forward);
        }

        if (!m_terminate)
            emit CycleCompleted(cycle + 1);
    }

    double totalSec = timer.elapsed() / 1000.0;
    emit testResults(
        m_params.regulatory_sequence,
        quint16(cycles),
        totalSec
        );
}

void CyclicTestSolenoid::processShutoff()
{
    const quint32 cycles = m_params.shutoff_numCycles;
    const quint32 delayMs = m_params.shutoff_delaySec * 1000;
    const quint32 holdMs = m_params.shutoff_holdTimeSec * 1000;
    const auto& raw = m_rawValuesOff;

    if (raw.isEmpty() || delayMs == 0 || cycles == 0)
        return;

    QVector<bool> doMask(m_params.shutoff_DO.begin(), m_params.shutoff_DO.end());
    int DO_COUNT = doMask.size();
    QVector<bool> currentStates(DO_COUNT, false);
    QVector<int> onCounts(DO_COUNT, 0), offCounts(DO_COUNT, 0);

    QElapsedTimer timer; timer.start();

    for (quint32 cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < raw.size() && !m_terminate; ++i) {
            if (i != 0) {
                for (int d = 0; d < DO_COUNT; ++d) {
                    if (!doMask[d]) continue;
                    currentStates[d] = !currentStates[d];
                    currentStates[d] ? ++onCounts[d] : ++offCounts[d];
                }
                emit SetMultipleDO(currentStates);
            }

            quint16 dacRaw = raw.at(i);
            SetDACBlocked(dacRaw, holdMs,
                          /*wait_for_stop=*/true,
                          /*wait_for_start=*/false);
            if (m_terminate) return;

            Sleep(delayMs);
            if (m_terminate) return;
        }

        if (!m_terminate)
            emit CycleCompleted(cycle + 1);
    }

    double totalSec = timer.elapsed() / 1000.0;
    emit testResults(
        m_params.shutoff_sequence,
        quint16(cycles),
        totalSec
        );
    emit DOCounts(onCounts, offCounts);
}
