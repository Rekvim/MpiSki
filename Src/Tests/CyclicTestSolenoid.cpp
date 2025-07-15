// CyclicTestSolenoid.cpp

#include "CyclicTestSolenoid.h"
#include <QThread>
#include <QElapsedTimer>

CyclicTestSolenoid::CyclicTestSolenoid(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false)
{ }

void CyclicTestSolenoid::SetParameters(const Parameters& params)
{
    m_params = params;
    parseSequence(params.regulatory_sequence, m_valuesReg);
    parseSequence(params.shutoff_sequence,    m_valuesOff);
}

void CyclicTestSolenoid::Process()
{
    // 1) Сбросить и запустить граф-таймер
    emit SetStartTime();
    emit ClearGraph();
    m_graphTimer->start(50);

    // 2) В зависимости от типа теста — запустить нужную логику
    switch (m_params.testType) {
    case Parameters::Regulatory:
        processRegulatory();
        break;
    case Parameters::Shutoff:
        processShutoff();
        break;
    case Parameters::Combined:
        processRegulatory();
        if (!m_terminate)
            processShutoff();
        break;
    }

    // 3) Остановить граф-таймер и завершить тест
    m_graphTimer->stop();
    emit EndTest();
}

void CyclicTestSolenoid::processRegulatory()
{
    const int cycles  = m_params.regulatory_numCycles;
    const int delayMs = m_params.regulatory_delaySec   * 1000;
    const int holdMs  = m_params.regulatory_holdTimeSec * 1000;
    const auto &values  = m_valuesReg;

    if (values.isEmpty() || delayMs <= 0 || holdMs < 0 || cycles <= 0)
        return;

    QElapsedTimer timer;
    timer.start();

    for (int cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < values.size() && !m_terminate; ++i) {
            int pct     = values[i];
            int prevPct = (i == 0 ? values.last() : values[i - 1]);
            bool forward = (pct > prevPct);

            SetDACBlocked(pct, delayMs);
            Sleep(delayMs);
            if (m_terminate) return;

            Sleep(holdMs);
            if (m_terminate) return;

            emit RegulatoryMeasurement(cycle, i, forward);
        }

        if (!m_terminate)
            emit CycleCompleted(cycle + 1);
    }

    // Итоговая статистика по времени
    double totalSec = timer.elapsed() / 1000.0;
    emit SetSolenoidResults(
        m_params.regulatory_sequence,
        quint16(cycles),
        totalSec
        );
}

void CyclicTestSolenoid::processShutoff()
{
    const int    cycles  = m_params.shutoff_numCycles;
    const int    delayMs = m_params.shutoff_delaySec   * 1000;
    const int    holdMs  = m_params.shutoff_holdTimeSec * 1000;
    const auto &values  = m_valuesOff;

    if (values.isEmpty() || delayMs <= 0 || holdMs < 0 || cycles <= 0)
        return;

    // Собираем маску DO и счётчики переключений
    QVector<bool> doMask(m_params.shutoff_DO.begin(), m_params.shutoff_DO.end());
    int DO_COUNT = doMask.size();
    QVector<bool> currentStates(DO_COUNT, false);
    QVector<int>  onCounts(DO_COUNT, 0), offCounts(DO_COUNT, 0);

    QElapsedTimer timer;
    timer.start();

    for (int cycle = 0; cycle < cycles && !m_terminate; ++cycle) {
        for (int i = 0; i < values.size() && !m_terminate; ++i) {
            // Инвертируем только выбранные DO
            for (int d = 0; d < DO_COUNT; ++d) {
                if (!doMask[d]) continue;
                currentStates[d] = !currentStates[d];
                currentStates[d] ? ++onCounts[d] : ++offCounts[d];
            }
            emit SetMultipleDO(currentStates);

            int pct = values[i];
            emit SetDAC(pct);

            Sleep(delayMs);
            if (m_terminate) return;

            Sleep(holdMs);
            if (m_terminate) return;
        }
    }

    double totalSec = timer.elapsed() / 1000.0;
    const QString seq = m_params.shutoff_enable_20mA
                            ? QStringLiteral("20mA")
                            : m_params.shutoff_sequence;
    emit SetSolenoidResults(seq,
                            quint16(cycles),
                            totalSec);
    emit DOCounts(onCounts, offCounts);
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
