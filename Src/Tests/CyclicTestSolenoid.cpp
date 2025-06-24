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

        const int N = values.size();
        QVector<QVector<double>> devLin(N), devPos(N);

        emit SetStartTime();
        emit ClearGraph();

        QElapsedTimer timer;
        timer.start();

        for (int c = 0; c < cycles && !m_terminate; ++c) {
            for (int i = 0; i < values.size() && !m_terminate; ++i) {
                int pct = values.at(i);
                int prevPct = (i == 0 ? values.first() : values.at(i - 1));

                quint64 t0 = timer.elapsed();
                emit TaskPoint(t0, prevPct);
                emit TaskPoint(t0, pct);

                emit SetDAC(pct);

                QThread::msleep(delayMs);

                quint64 t1 = timer.elapsed();
                emit TaskPoint(t1, pct);

                double measuredLin = m_mpi[0]->GetPersent();
                double measuredPos =  m_mpi.GetDAC()->GetValue();

                devLin[i].append(qAbs(measuredLin - pct));
                 double posPct = (measuredPos - 4.0) / 16.0 * 100.0;
                devPos[i].append(qAbs(posPct - pct));

                QThread::msleep(holdMs);
                quint64 t2 = timer.elapsed();
                emit TaskPoint(t2, pct);

                emit UpdateCyclicTred();
            }
        }

         double totalSec = timer.elapsed() / 1000.0;

         bool isShutoff = (&values == &m_valuesOff);
        QString seq = isShutoff && m_params.shutoff_enable_20mA
                          ? QStringLiteral("20mA")
                          : (isShutoff
                                 ? m_params.shutoff_sequence
                                 : m_params.regulatory_sequence);

        QVector<RangeDeviationRecord> recs(N);
        for (int i = 0; i < N; ++i) {
            const auto& L = devLin[i];
            const auto& P = devPos[i];

            // 1) среднее
            recs[i].avgErrorLinear     = std::accumulate(L.begin(), L.end(), 0.0) / L.size();
            recs[i].avgErrorPositioner = std::accumulate(P.begin(), P.end(), 0.0) / P.size();

            // 2) максимумы и их индексы
            auto itL  = std::max_element(L.begin(), L.end());
            recs[i].maxErrorLinear     = *itL;
            recs[i].maxErrorLinearCycle  = quint32(std::distance(L.begin(), itL)) + 1;

            auto itP  = std::max_element(P.begin(), P.end());
            recs[i].maxErrorPositioner    = *itP;
            recs[i].maxErrorPositionerCycle = quint32(std::distance(P.begin(), itP)) + 1;
        }

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
