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
        // --- валидация
        if (values.isEmpty() || delayMs <= 0 || holdMs < 0 || cycles <= 0) {
            emit EndTest();
            return;
        }

        // Старт измерения
        emit SetStartTime();
        emit ClearGraph();

        QElapsedTimer timer;
        timer.start();

        // Основной цикл
        for (int c = 0; c < cycles && !m_terminate; ++c) {
            for (int i = 0; i < values.size() && !m_terminate; ++i) {
                int pct     = values.at(i);
                int prevPct = (i == 0 ? values.first() : values.at(i - 1));

                // точка «до хода»
                quint64 t0 = timer.elapsed();
                emit TaskPoint(t0, prevPct);
                emit TaskPoint(t0, pct);

                // команда на аппарат
                emit SetDAC(pct);

                // ждём delayMs
                QThread::msleep(delayMs);

                // помечаем окончание хода
                quint64 t1 = timer.elapsed();
                emit TaskPoint(t1, pct);

                // удерживаем
                QThread::msleep(holdMs);
                quint64 t2 = timer.elapsed();
                emit TaskPoint(t2, pct);

                // обновляем тренд
                emit UpdateCyclicTred();
            }
        }

        // рассчитаем общее время
        double totalSec = timer.elapsed() / 1000.0;

        // какую строку выводить
        bool isShutoff = (&values == &m_valuesOff);
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
        if (m_params.shutoff_enable_20mA) {
            emit SetStartTime();
            emit TaskPoint(0, 100);
            emit SetDAC(100);
            QThread::msleep(m_params.shutoff_delaySec * 1000);
        }
        // 2) а потом основную последовательность
        runLoop(m_valuesOff,
                m_params.shutoff_delaySec * 1000,
                m_params.shutoff_holdTimeSec * 1000,
                m_params.shutoff_numCycles);
    }

    void CyclicTestSolenoid::processCombined()
    {
        processRegulatory();
        if (!m_terminate)
            processShutoff();
    }
