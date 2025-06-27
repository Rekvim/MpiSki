    // CyclicTestSolenoid.cpp
    #include "CyclicTestSolenoid.h"
    #include <QtGlobal>     // qAbs()

    CyclicTestSolenoid::CyclicTestSolenoid(QObject* parent)
        : MainTest(parent, /*isCyclic=*/false)
    {}

    void CyclicTestSolenoid::SetParameters(const Parameters& params)
    {
        m_params = params;
        parseSequence(params.regulatory_sequence, m_valuesReg);
        parseSequence(params.shutoff_sequence, m_valuesOff);
    }

    void CyclicTestSolenoid::Process()
    {
        qDebug() << "[CTS] Process() start in thread" << QThread::currentThread();
        switch (m_params.testType) {
        case Parameters::Regulatory: processRegulatory(); break;
        case Parameters::Shutoff:    processShutoff();    break;
        case Parameters::Combined:   processCombined();   break;
        }
        qDebug() << "[CTS] Process() end";
        emit EndTest();
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

    void CyclicTestSolenoid::runLoop(const QVector<int>& values,
                                     int delayMs,
                                     int holdMs,
                                     int cycles)
    {
        if (values.isEmpty() || delayMs <= 0 || holdMs < 0 || cycles <= 0) {
            emit EndTest();
            return;
        }

        const bool isShutoff = (&values == &m_valuesOff);
        const int N = values.size();

        emit SetStartTime();
        emit ClearGraph();
        QElapsedTimer timer; timer.start();

        for (int c = 0; c < cycles && !m_terminate; ++c) {
            for (int i = 0; i < N && !m_terminate; ++i) {
                int pct = values[i];
                int prevPct = (i == 0 ? values.first() : values[i-1]);
                bool forward = (pct > prevPct);

                quint64 t0 = timer.elapsed();
                emit TaskPoint(t0, prevPct);
                emit TaskPoint(t0, pct);

                if (isShutoff) {
                    for (quint8 d = 0; d < 4; ++d)
                        if (m_params.shutoff_DO[d])
                            emit SetDO(d, true);
                }

                emit SetDAC(pct);
                QThread::msleep(delayMs);

                quint64 t1 = timer.elapsed();
                emit TaskPoint(t1, pct);

                QThread::msleep(holdMs);

                if (isShutoff) {
                    for (quint8 d = 0; d < 4; ++d)
                        if (m_params.shutoff_DO[d])
                            emit SetDO(d, false);
                }

                quint64 t2 = timer.elapsed();
                emit TaskPoint(t2, pct);
                emit UpdateCyclicTred();

                if (&values == &m_valuesReg) {
                    emit RegulatoryMeasurement(c, i, forward);
                }
            }
        }

        double totalSec = timer.elapsed() / 1000.0;

        QString seq = isShutoff && m_params.shutoff_enable_20mA
                          ? QStringLiteral("20mA")
                          : (isShutoff
                                 ? m_params.shutoff_sequence
                                 : m_params.regulatory_sequence);

        emit SetSolenoidResults(seq,
                                quint16(cycles),
                                totalSec);

        emit EndTest();
    }

    void CyclicTestSolenoid::processRegulatory()
    {
        runLoop(
            m_valuesReg,
            m_params.regulatory_delaySec   * 1000,
            m_params.regulatory_holdTimeSec * 1000,
            m_params.regulatory_numCycles
            );
    }

    void CyclicTestSolenoid::processShutoff()
    {
        // 1) предтест 20 mA (без изменений) …
        // if (m_params.shutoff_enable_20mA) {
        //     emit SetStartTime();
        //     emit TaskPoint(0, 100);
        //     emit SetDAC(100);
        //     QThread::msleep(m_params.shutoff_delaySec * 1000);
        // }

        // 2) Собираем маску тех DO, которые пользователь включил:
        QVector<bool> doMask(m_params.shutoff_DO.begin(), m_params.shutoff_DO.end());
        int DO_COUNT = doMask.size();

        // 3) Текущее состояние (начинаем с false) и счётчики переключений:
        QVector<bool> currentStates(DO_COUNT, false);
        QVector<int>  onCounts(DO_COUNT, 0), offCounts(DO_COUNT, 0);

        const int delayMs = m_params.shutoff_delaySec   * 1000;
        const int holdMs  = m_params.shutoff_holdTimeSec * 1000;
        const int cycles  = m_params.shutoff_numCycles;
        const int N       = m_valuesOff.size();

        emit SetStartTime();
        emit ClearGraph();
        QElapsedTimer timer; timer.start();

        for (int c = 0; c < cycles && !m_terminate; ++c) {
            for (int i = 0; i < N && !m_terminate; ++i) {
                int pct     = m_valuesOff[i];
                int prevPct = (i == 0 ? m_valuesOff.first() : m_valuesOff[i-1]);

                // график
                quint64 t0 = timer.elapsed();
                emit TaskPoint(t0, prevPct);
                emit TaskPoint(t0, pct);

                // единственная инверсия состояний выбранных DO
                for (int d = 0; d < DO_COUNT; ++d) {
                    if (!doMask[d]) continue;
                    currentStates[d] = !currentStates[d];
                    if (currentStates[d])
                        ++onCounts[d];
                    else
                        ++offCounts[d];
                }
                emit SetMultipleDO(currentStates);

                emit SetDAC(pct);
                QThread::msleep(delayMs);

                quint64 t1 = timer.elapsed();
                emit TaskPoint(t1, pct);

                QThread::msleep(holdMs);

                quint64 t2 = timer.elapsed();
                emit TaskPoint(t2, pct);

                emit UpdateCyclicTred();
            }
        }

        // итоги
        double totalSec = timer.elapsed() / 1000.0;
        QString seq = m_params.shutoff_enable_20mA
                          ? QStringLiteral("20mA")
                          : m_params.shutoff_sequence;
        emit SetSolenoidResults(seq, quint16(cycles), totalSec);

        // отдадим счётчики переключений и завершим тест
        emit DOCounts(onCounts, offCounts);
        emit EndTest();
    }
    void CyclicTestSolenoid::processCombined()
    {
        processRegulatory();
        if (!m_terminate)
            processShutoff();
    }
