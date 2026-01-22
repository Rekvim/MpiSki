#include "CyclicTestsShutoff.h"

CyclicTestsShutoff::CyclicTestsShutoff(QObject* parent)
    : MainTest(parent, /*isCyclic=*/false)
{}

void CyclicTestsShutoff::SetTask(const Task& task)
{
    m_task = task;
}

static inline bool bit(quint8 mask, int b)
{
    return (mask & (1u << b)) != 0;
}

void CyclicTestsShutoff::Process()
{
    emit ClearGraph();

    emit SetStartTime();
    m_graphTimer->start(100);

    const int DO_COUNT = m_task.doMask.size();
    m_doOnCounts.assign(DO_COUNT, 0);
    m_doOffCounts.assign(DO_COUNT, 0);

    // 1) Стартовое состояние DO берём из железа (важно для корректного toggle)
    quint8 hwDoMask = 0;
    emit GetDO(hwDoMask);

    QVector<bool> currentStates(DO_COUNT, false);
    for (int d = 0; d < DO_COUNT; ++d) {
        currentStates[d] = bit(hwDoMask, d);
    }

    // 2) Подготовка DI-подсчёта
    quint8 lastDI = 0;
    emit GetDI(lastDI);

    int s3to0 = 0; // "closed" rising edges (бит 0: 0->1)
    int s0to3 = 0; // "open"   rising edges (бит 1: 0->1)

    auto consumeDI = [&](quint8 nowDI) {
        const bool lastClosed = bit(lastDI, 0);
        const bool lastOpen   = bit(lastDI, 1);
        const bool nowClosed  = bit(nowDI, 0);
        const bool nowOpen    = bit(nowDI, 1);

        // считаем только сработки (rising edges)
        if (nowClosed && !lastClosed) ++s3to0;
        if (nowOpen   && !lastOpen)   ++s0to3;

        lastDI = nowDI;
    };

    const int perCycle = (m_task.cycles > 0
                              ? int(m_task.values.size() / m_task.cycles)
                              : int(m_task.values.size()));

    for (quint32 step = 0; step < quint32(m_task.values.size()) && !m_terminate; ++step) {
        const quint16 value = m_task.values.at(int(step));

        // Переключаем DO (как у тебя), но DI считаем по факту после воздействия
        if (perCycle > 0 && (step % quint32(perCycle)) != 0) {
            for (int d = 0; d < DO_COUNT; ++d) {
                if (!m_task.doMask[d]) continue;

                currentStates[d] = !currentStates[d];

                if (currentStates[d]) ++m_doOnCounts[d];
                else ++m_doOffCounts[d];
            }

            emit SetMultipleDO(currentStates);

            // DI сразу после переключения DO (иногда уже успевает измениться)
            quint8 di = 0;
            emit GetDI(di);
            consumeDI(di);
        }

        setDacBlocked(value, m_task.delayMsecs);
        if (m_terminate) { emit EndTest(); return; }

        setDacBlocked(value, m_task.holdMsecs);
        if (m_terminate) { emit EndTest(); return; }

        // DI после выдержки — обычно самое надёжное место для фиксации сработки
        {
            quint8 di = 0;
            emit GetDI(di);
            consumeDI(di);
        }

        if (perCycle > 0 && (step + 1) % quint32(perCycle) == 0) {
            emit CycleCompleted(int((step + 1) / quint32(perCycle)));
        }
    }

    setDacBlocked(0, 0, true);
    m_graphTimer->stop();

    TestResults r;
    r.numCycles = m_task.cycles;
    r.doOnCounts = m_doOnCounts;
    r.doOffCounts = m_doOffCounts;
    r.switch3to0Count = quint16(s3to0);
    r.switch0to3Count = quint16(s0to3);

    emit Results(r);
    emit EndTest();
}
