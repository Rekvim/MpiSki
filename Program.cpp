    #include "Program.h"

// #include "./Src/Tests/CyclicTestPositioner.h"
#include "./Src/Tests/CyclicTestSolenoid.h"
#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Src/Tests/MainTest.h"

Program::Program(QObject *parent)
    : QObject{parent}
{
    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);

    m_dacEventloop = new QEventLoop(this);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::UpdateSensors);

    m_testing = false;

    m_timerDI = new QTimer(this);
    m_timerDI->setInterval(1000);
    connect(m_timerDI, &QTimer::timeout, this, [&]() {
        quint8 DI = m_mpi.GetDIStatus();
        emit SetCheckboxDIChecked(DI);
    });

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::UpdateCharts_CyclicSolenoid);

}

void Program::SetRegistry(Registry *registry)
{
    m_registry = registry;
}

void Program::SetDAC(quint16 dac, quint32 sleep_ms, bool wait_for_stop, bool wait_for_start)
{
    m_stopSetDac = false;

    if (m_mpi.SensorCount() == 0) {
        emit ReleaseBlock();
        return;
    }

    m_mpi.SetDAC_Raw(dac);

    if (wait_for_start) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->GetRawValue());
            if (qAbs(lineSensor.first() - lineSensor.last()) > 10) {
                timer.stop();
                m_dacEventloop->quit();
            }
            if (lineSensor.size() > 50) {
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) {
        emit ReleaseBlock();
        return;
    }

    if (sleep_ms > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout, m_dacEventloop, &QEventLoop::quit);
        timer.start(sleep_ms);
        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) {
        emit ReleaseBlock();
        return;
    }

    if (wait_for_stop) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->GetRawValue());
            if (lineSensor.size() == 50) {
                if (qAbs(lineSensor.first() - lineSensor.last()) < 10) {
                    timer.stop();
                    m_dacEventloop->quit();
                }
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventloop->exec();
        timer.stop();
    }

    emit ReleaseBlock();
}

void Program::SetTimeStart()
{
    m_startTime = QDateTime::currentMSecsSinceEpoch();
}

void Program::AddRegression(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;
    for (QPointF point : points) {
        chartPoints.push_back({1, point.x(), point.y()});
    }
    emit AddPoints(Charts::Pressure, chartPoints);

    //emit SetVisible(Charts::Pressure, 1, true);
    emit SetRegressionEnable(true);
}

void Program::AddFriction(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;

    ValveInfo *valveInfo = m_registry->GetValveInfo();

    qreal k = 5 * M_PI * valveInfo->driveDiameter * valveInfo->driveDiameter / 4;

    for (QPointF point : points) {
        chartPoints.push_back({0, point.x(), point.y() * k});
    }
    emit AddPoints(Charts::Friction, chartPoints);
}

void Program::UpdateSensors()
{
    for (quint8 i = 0; i < m_mpi.SensorCount(); ++i) {
        switch (i) {
        case 0:
            emit SetText(TextObjects::LineEdit_linearSensor, m_mpi[i]->GetFormatedValue());
            emit SetText(TextObjects::LineEdit_linearSensorPercent, m_mpi[i]->GetPersentFormated());
            break;
        case 1:
            emit SetText(TextObjects::LineEdit_pressureSensor_1, m_mpi[i]->GetFormatedValue());
            break;
        case 2:
            emit SetText(TextObjects::LineEdit_pressureSensor_2, m_mpi[i]->GetFormatedValue());
            break;
        case 3:
            emit SetText(TextObjects::LineEdit_pressureSensor_3, m_mpi[i]->GetFormatedValue());
            break;
        }
    }

    if (m_testing)
        emit SetTask(m_mpi.GetDAC()->GetValue());


    Sensor *feedbackSensor = m_mpi.GetDAC();
    QString fbValue = feedbackSensor->GetFormatedValue();
    emit SetText(TextObjects::LineEdit_feedback_4_20mA, fbValue);

    QVector<Point> points;
    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_initTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::Trend, points);
}



void Program::StepTestResults(QVector<StepTest::TestResult> results, quint32 T_value)
{
    m_telemetryStore.stepResults.clear();
    for (auto &r : results) {
        StepTestRecord rec;
        rec.from = r.from;
        rec.to = r.to;
        rec.T_value = r.T_value;
        rec.overshoot = r.overshoot;
        m_telemetryStore.stepResults.push_back(rec);
    }

    emit TelemetryUpdated(m_telemetryStore);

    emit SetStepResults(results, T_value);
}

void Program::EndTest()
{
    m_testing = false;
    emit EnableSetTask(true);
    emit SetButtonInitEnabled(true);

    if (!m_cyclicRunning) {
        SetDAC_int(0);
    }

    if (m_diPollTimer && m_diPollTimer->isActive()) {
        m_diPollTimer->stop();
    }

    emit StopTest();
    emit SetTask(m_mpi.GetDAC()->GetValue());

    m_cyclicRunning = false;
}

void Program::SetDAC_real(qreal value)
{
    m_mpi.SetDAC_Real(value);
}

void Program::SetDAC_int(quint16 value)
{
    qreal mA = 4.0 + (16.0 * value / 100.0);

    quint16 raw = m_mpi.GetDAC()->GetRawFromValue(mA);


    m_mpi.SetDAC_Raw(raw);


    emit ReleaseBlock();
}
void Program::SetBlockCTS(const SelectTests::BlockCTS &blockCTS)
{
    m_blockCts = blockCTS;
}
void Program::button_init()
{
    m_timerSensors->stop();
    m_timerDI->stop();

    emit SetButtonInitEnabled(false);
    emit SetSensorNumber(0);

    auto &s = m_telemetryStore;

    s.init.deviceStatusText = m_mpi.Connect()
                                  ? QString("Успешное подключение к порту %1").arg(m_mpi.PortName())
                                  : "Ошибка подключения";
    s.init.deviceStatusColor = m_mpi.Connect() ? Qt::darkGreen : Qt::red;

    emit TelemetryUpdated(s);

    s.init.initStatusText = m_mpi.Initialize()
                                ? "Успешная инициализация"
                                : "Ошибка инициализации";
    s.init.initStatusColor = m_mpi.Initialize() ? Qt::darkGreen : Qt::red;

    emit TelemetryUpdated(s);

    if ((m_mpi.Version() & 0x40) != 0) {
        emit SetButtonsDOChecked(m_mpi.GetDOStatus());
        m_timerDI->start();
    }

    m_isInitialized = true;

    if (m_mpi.SensorCount() == 0) {
        s.init.connectedSensorsText = "Датчики не обнаружены";
        s.init.connectedSensorsColor = Qt::red;
    } else if (m_mpi.SensorCount() == 1) {
        s.init.connectedSensorsText = "Обнаружен 1 датчик";
        s.init.connectedSensorsColor = Qt::darkYellow;
    } else {
        s.init.connectedSensorsText = QString("Обнаружено %1 датчика").arg(m_mpi.SensorCount());
        s.init.connectedSensorsColor = Qt::darkGreen;
    }

    emit TelemetryUpdated(s);

    ValveInfo *valveInfo = m_registry->GetValveInfo();

    s.init.startingPositionText = "Измерение";
    s.init.startingPositionColor = Qt::darkYellow;

    emit TelemetryUpdated(s);

    SetDAC(0, 10000, true);

    QTimer timer(this);
    connect(&timer, &QTimer::timeout, this, [&] {
        if (!m_waitForButton || m_stopSetDac) {
            m_dacEventloop->quit();
        }
    });

    timer.start(50);
    m_dacEventloop->exec();
    timer.stop();

    bool normalClosed = (valveInfo->safePosition == 0);
    if (normalClosed) m_mpi[0]->SetMin();
    else m_mpi[0]->SetMax();

    s.init.startingPositionText = m_mpi[0]->GetFormatedValue();
    s.init.startingPositionColor = Qt::darkGreen;

    s.init.finalPositionText = "Измерение";
    s.init.finalPositionColor = Qt::darkYellow;

    emit TelemetryUpdated(s);

    SetDAC(0xFFFF, 10000, true);

    timer.start(50);
    m_dacEventloop->exec();
    timer.stop();

    if (normalClosed) m_mpi[0]->SetMax();
    else m_mpi[0]->SetMin();

    s.init.finalPositionText = m_mpi[0]->GetFormatedValue();
    s.init.finalPositionColor = Qt::darkGreen;

    emit TelemetryUpdated(s);

    qreal correctCoefficient = 1;
    if (valveInfo->strokeMovement != 0) {
        correctCoefficient = qRadiansToDegrees(2 / valveInfo->diameterPulley);
        m_mpi[0]->SetUnit("°");
    }
    m_mpi[0]->CorrectCoefficients(correctCoefficient);

    if (normalClosed) {
        m_telemetryStore.valveStrokeRecord.range = m_mpi[0]->GetFormatedValue();
        m_telemetryStore.valveStrokeRecord.real = m_mpi[0]->GetValue();
        SetDAC(0);
    } else {
        SetDAC(0, 10000, true);
        m_telemetryStore.valveStrokeRecord.range = m_mpi[0]->GetFormatedValue();
        m_telemetryStore.valveStrokeRecord.real = m_mpi[0]->GetValue();
    }
    emit TelemetryUpdated(s);

    emit SetTask(m_mpi.GetDAC()->GetValue());

    emit ClearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    emit SetSensorNumber(m_mpi.SensorCount());
    emit SetButtonInitEnabled(true);
    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}

void Program::MainTestStart()
{
    MainTestSettings::TestParameters parameters;
    emit GetMainTestParameters(parameters);

    if (parameters.delay == 0) {
        emit StopTest();
        return;
    }

    parameters.dac_min = qMax(m_mpi.GetDAC()->GetRawFromValue(parameters.signal_min),
                              m_mpi.GetDac_Min());
    parameters.dac_max = qMin(m_mpi.GetDAC()->GetRawFromValue(parameters.signal_max),
                              m_mpi.GetDac_Max());

    emit SetButtonInitEnabled(false);

    MainTest *mainTest = new MainTest;

    mainTest->SetParameters(parameters);

    QThread *threadTest = new QThread(this);
    mainTest->moveToThread(threadTest);

    connect(threadTest, &QThread::started,
            mainTest, &MainTest::Process);

    connect(mainTest, &MainTest::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::StopTest,
            mainTest, &MainTest::Stop);

    connect(threadTest, &QThread::finished,
            threadTest, &QThread::deleteLater);

    connect(threadTest, &QThread::finished,
            mainTest, &MainTest::deleteLater);

    connect(mainTest, &MainTest::EndTest,
            this, &Program::EndTest);

    connect(mainTest, &MainTest::EndTest,
            this, &Program::MainTestFinished);

    connect(mainTest, &MainTest::UpdateGraph,
            this, &Program::UpdateCharts_mainTest);

    connect(mainTest, &MainTest::SetDAC,
            this, &Program::SetDAC);

    connect(mainTest, &MainTest::DublSeries,
            this, [&] { emit DublSeries(); });

    connect(mainTest, &MainTest::GetPoints,
            this, &Program::GetPoints_mainTest,
            Qt::BlockingQueuedConnection);

    connect(mainTest, &MainTest::AddRegression,
            this, &Program::AddRegression);

    connect(mainTest, &MainTest::AddFriction,
            this, &Program::AddFriction);

    connect(this, &Program::ReleaseBlock,
            mainTest, &MainTest::ReleaseBlock);

    connect(mainTest, &MainTest::Results,
            this, &Program::MainTestResults);

    connect(mainTest, &MainTest::ShowDots,
            this, [&](bool visible) { emit ShowDots(visible); });

    connect(mainTest, &MainTest::ClearGraph, this, [&] {
        emit ClearPoints(Charts::Task);
        emit ClearPoints(Charts::Pressure);
        emit ClearPoints(Charts::Friction);
        emit SetRegressionEnable(false);
    });

    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}

void Program::MainTestResults(MainTest::TestResults results)
{
    ValveInfo *valveInfo = m_registry->GetValveInfo();

    qreal k = 5 * M_PI * valveInfo->driveDiameter * valveInfo->driveDiameter / 4;

    auto &s = m_telemetryStore.mainTestRecord;

    s.pressureDifference = results.pressureDiff;

    s.frictionForce = results.pressureDiff * k;
    s.frictionPercent = results.friction;

    s.dynamicError_mean = results.dynamicErrorMean;
    s.dynamicError_meanPercent = results.dynamicErrorMean / 0.16;
    s.dynamicError_max = results.dynamicErrorMax;
    s.dynamicError_maxPercent = results.dynamicErrorMean / 0.16;
    s.dynamicErrorReal = results.dynamicErrorMean / 0.16;

    s.lowLimitPressure = results.lowLimitPressure;
    s.highLimitPressure = results.highLimitPressure;

    s.springLow = results.springLow;
    s.springHigh = results.springHigh;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::GetPoints_mainTest(QVector<QVector<QPointF>> &points)
{
    emit GetPoints(points, Charts::Task);
}

void Program::UpdateCharts_mainTest()
{
    QVector<Point> points;
    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    qreal task = m_mpi[0]->GetValueFromPercent(percent);
    qreal X = m_mpi.GetDAC()->GetValue();
    points.push_back({0, X, task});

    for (quint8 i = 0; i < m_mpi.SensorCount(); ++i) {
        points.push_back({static_cast<quint8>(i + 1), X, m_mpi[i]->GetValue()});
    }

    emit AddPoints(Charts::Task, points);

    points.clear();
    points.push_back({0, m_mpi[1]->GetValue(), m_mpi[0]->GetValue()});

    emit AddPoints(Charts::Pressure, points);
}

void Program::StrokeTestStart()
{
    emit SetButtonInitEnabled(false);
    emit ClearPoints(Charts::Stroke);

    StrokeTest *strokeTest = new StrokeTest;
    QThread *threadTest = new QThread(this);
    strokeTest->moveToThread(threadTest);

    connect(threadTest, &QThread::started, strokeTest, &StrokeTest::Process);
    connect(strokeTest, &StrokeTest::EndTest, threadTest, &QThread::quit);

    connect(this, &Program::StopTest, strokeTest, &StrokeTest::Stop);
    connect(threadTest, &QThread::finished, threadTest, &QThread::deleteLater);
    connect(threadTest, &QThread::finished, strokeTest, &StrokeTest::deleteLater);

    connect(this, &Program::ReleaseBlock, strokeTest, &MainTest::ReleaseBlock);

    connect(strokeTest, &StrokeTest::EndTest, this, &Program::EndTest);

    connect(strokeTest, &StrokeTest::UpdateGraph, this, &Program::UpdateCharts_strokeTest);
    connect(strokeTest, &StrokeTest::SetDAC, this, &Program::SetDAC);
    connect(strokeTest, &StrokeTest::SetStartTime, this, &Program::SetTimeStart);
    connect(strokeTest, &StrokeTest::Results, this, &Program::StrokeTestResults);

    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}


void Program::StrokeTestResults(quint64 forwardTime, quint64 backwardTime)
{

    m_telemetryStore.strokeTestRecord.timeForwardMs  = forwardTime;
    m_telemetryStore.strokeTestRecord.timeBackwardMs = backwardTime;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::UpdateCharts_strokeTest()
{
    QVector<Point> points;

    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::Stroke, points);
}

void Program::UpdateCharts_CyclicSolenoid()
{
    if (!m_cyclicRunning)
        return;

    qreal task = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    task = qBound<qreal>(0.0, task, 100.0);


    qreal measured = m_mpi[0]->GetPersent();


    quint64 t = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    QVector<Point> pts;

    pts.push_back({0, qreal(t), task});
    pts.push_back({1, qreal(t), measured});

    emit AddPoints(Charts::CyclicSolenoid, pts);
}

void Program::pollDIForCyclic()
{
    if (!m_cyclicRunning)
        return;

    quint8 di = m_mpi.GetDIStatus();
    if (di == m_lastDI)
        return;

    quint64 t = QDateTime::currentMSecsSinceEpoch() - m_cyclicStartTs;

    QVector<Point> pts;
    if ((di & 0x01) && !(m_lastDI & 0x01)) {
        ++m_telemetryStore.cyclicTestRecord.switch3to0Count;
        pts.push_back({2, qreal(t), 100.0});
    }
    if ((di & 0x02) && !(m_lastDI & 0x02)) {
        ++m_telemetryStore.cyclicTestRecord.switch0to3Count;
        pts.push_back({3, qreal(t), 100.0});
    }

    if (!pts.isEmpty())
        emit AddPoints(Charts::CyclicSolenoid, pts);

    m_lastDI = di;
}

void Program::SetMultipleDO(const QVector<bool>& states)
{
    quint8 mask = 0;
    for (int d = 0; d < states.size(); ++d) {
        m_mpi.SetDiscreteOutput(d, states[d]);
        if (states[d]) mask |= (1 << d);
    }
    emit SetButtonsDOChecked(mask);
}
void Program::CyclicSolenoidTestStart(const CyclicTestSettings::TestParameters &p)
{
    using TP = CyclicTestSettings::TestParameters;

    m_cyclicRunning = true;

    bool ok = true;
    if (p.testType == TP::Regulatory || p.testType == TP::Combined) {
        ok &= !p.regulatory_sequence.isEmpty()
        && p.regulatory_delaySec > 0
        && p.regulatory_holdTimeSec >= 0
        && p.regulatory_numCycles > 0;
    }
    if (p.testType == TP::Shutoff || p.testType == TP::Combined) {
        ok &= !p.shutoff_sequence.isEmpty()
        && p.shutoff_delaySec > 0
        && p.shutoff_holdTimeSec >= 0
        && p.shutoff_numCycles > 0;
    }
    if (!ok) {
        emit StopTest();
        return;
    }

    const auto parts = p.regulatory_sequence.split('-', Qt::SkipEmptyParts);
    int recordCount = parts.size();
    QVector<int> valuesReg; valuesReg.reserve(recordCount);
    for (const QString &part : parts) {
        bool convOk = false;
        int v = part.trimmed().toInt(&convOk);
        if (convOk) valuesReg.append(v);
    }

    m_telemetryStore.cyclicTestRecord.ranges.clear();
    m_telemetryStore.cyclicTestRecord.ranges.resize(recordCount);
    for (int i = 0; i < recordCount; ++i) {
        auto &rec = m_telemetryStore.cyclicTestRecord.ranges[i];
        rec.rangePercent     = valuesReg[i];
        rec.maxForwardValue  = 0;
        rec.maxForwardCycle  = 0;
        rec.maxReverseValue  = 0;
        rec.maxReverseCycle  = 0;
    }
    m_telemetryStore.cyclicTestRecord.switch3to0Count = 0;
    m_telemetryStore.cyclicTestRecord.switch0to3Count = 0;
    m_lastDI = m_mpi.GetDIStatus();
    m_cyclicStartTs = QDateTime::currentMSecsSinceEpoch();

    m_diPollTimer = new QTimer(this);
    m_diPollTimer->setInterval(50);
    connect(m_diPollTimer, &QTimer::timeout,
            this, &Program::pollDIForCyclic);
    m_diPollTimer->start();

    auto *sol = new CyclicTestSolenoid;
    sol->SetParameters(p);

    QThread *thr = new QThread;
    sol->moveToThread(thr);

    connect(thr, &QThread::started,
            sol, &CyclicTestSolenoid::Process,
            Qt::QueuedConnection);

    connect(this, &Program::StopTest,
            sol, &CyclicTestSolenoid::Stop,
            Qt::QueuedConnection);

    connect(sol, &CyclicTestSolenoid::EndTest,
            thr, &QThread::quit,
            Qt::QueuedConnection);
    connect(sol, &CyclicTestSolenoid::EndTest,
            this, &Program::EndTest,
            Qt::QueuedConnection);

    connect(thr, &QThread::finished,
            sol, &QObject::deleteLater,
            Qt::QueuedConnection);

    connect(thr, &QThread::finished,
            thr, &QObject::deleteLater,
            Qt::QueuedConnection);

    connect(sol, &CyclicTestSolenoid::RegulatoryMeasurement,
            this, [this](int cycle, int step, bool forward) {
                auto &rec = m_telemetryStore.cyclicTestRecord.ranges[step];
                qreal measured = m_mpi[0]->GetPersent();
                if (forward) {
                    if (measured > rec.maxForwardValue) {
                        rec.maxForwardValue = measured;
                        rec.maxForwardCycle = cycle;
                    }
                } else {
                    if (measured > rec.maxReverseValue) {
                        rec.maxReverseValue = measured;
                        rec.maxReverseCycle = cycle;
                    }
                }
            });

    connect(sol, &CyclicTestSolenoid::DOCounts,
            this, &Program::onDOCounts);

    connect(sol, &CyclicTestSolenoid::SetMultipleDO,
            this, &Program::SetMultipleDO);

    connect(sol, &CyclicTestSolenoid::SetDO,
            this, &Program::button_DO);

    connect(sol, &CyclicTestSolenoid::ClearGraph,
            this, [&] {
                emit ClearPoints(Charts::CyclicSolenoid);
            });

    connect(sol, &CyclicTestSolenoid::TaskPoint,
            this, [this](quint64 t, int pct) {
                if (m_cyclicRunning) {
                    emit AddPoints(Charts::CyclicSolenoid,
                                   QVector<Point>{{0, qreal(t), qreal(pct)}});
                }
            });

    connect(sol, &CyclicTestSolenoid::SetStartTime,
            this, &Program::SetTimeStart);

    connect(sol, &CyclicTestSolenoid::SetSolenoidResults,
            this, &Program::SolenoidResults);

    connect(sol, &CyclicTestSolenoid::SetDAC,
            this, &Program::SetDAC_int);

    connect(this, &Program::ReleaseBlock,
            sol, &CyclicTestSolenoid::ReleaseBlock);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::UpdateCharts_CyclicSolenoid,
            Qt::UniqueConnection);

    emit ClearPoints(Charts::CyclicSolenoid);
    m_testing = true;
    emit EnableSetTask(false);

    thr->start();
    m_timerSensors->start();
}

void Program::onDOCounts(const QVector<int>& on, const QVector<int>& off) {
    m_telemetryStore.cyclicTestRecord.doOnCounts = on;
    m_telemetryStore.cyclicTestRecord.doOffCounts = off;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::SolenoidResults(QString sequence,
                              quint16 cycles,
                              double totalTimeSec)
{

    m_telemetryStore.cyclicTestRecord.sequence = sequence;
    m_telemetryStore.cyclicTestRecord.cycles = cycles;
    m_telemetryStore.cyclicTestRecord.totalTimeSec = totalTimeSec;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::StartOptionalTest(quint8 testNum)
{
    OptionTest::Task task;
    OptionTest *optionalTest;

    switch (testNum) {
    case 0: {
        optionalTest = new OptionTest;

        OtherTestSettings::TestParameters parameters;
        emit GetResponseTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valveInfo = m_registry->GetValveInfo();

        bool normalOpen = (valveInfo->safePosition != 0);

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(4.0));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            for (quint8 i = 0; i < 2; i++) {
                qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
                qreal dacValue = m_mpi.GetDAC()->GetRawFromValue(current);
                task.value.push_back(dacValue);

                for (auto it_s = parameters.steps.begin(); it_s < parameters.steps.end(); ++it_s) {
                    current += (16 * *it_s / 100) * (i == 0 ? 1 : -1) * (normalOpen ? -1 : 1);
                    dacValue = m_mpi.GetDAC()->GetRawFromValue(current);
                    task.value.push_back(dacValue);
                }
            }
        }

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(4.0));

        optionalTest->SetTask(task);

        connect(optionalTest, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optionTest(Charts::Response);
        });

        emit ClearPoints(Charts::Response);

        break;
    }
    case 1: {
        optionalTest = new OptionTest;
        OtherTestSettings::TestParameters parameters;
        emit GetResolutionTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valveInfo = m_registry->GetValveInfo();

        bool normalOpen = (valveInfo->safePosition != 0);

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(4.0));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
            qreal dacValue = m_mpi.GetDAC()->GetRawFromValue(current);

            for (auto it_s = parameters.steps.begin(); it_s < parameters.steps.end(); ++it_s) {
                task.value.push_back(dacValue);
                current = (16 * (normalOpen ? 100 - *it - *it_s : *it + *it_s) / 100 + 4.0);
                qreal dacValueStep = m_mpi.GetDAC()->GetRawFromValue(current);
                task.value.push_back(dacValueStep);
            }
        }

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(4.0));

        optionalTest->SetTask(task);

        connect(optionalTest, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optionTest(Charts::Resolution);
        });

        emit ClearPoints(Charts::Resolution);

        break;
    }

    case 2: {
        optionalTest = new StepTest;
        StepTestSettings::TestParameters parameters;
        emit GetStepTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;
        ValveInfo *valveInfo = m_registry->GetValveInfo();

        qreal startValue = 4.0;
        qreal endValue = 20.0;

        bool normalOpen = (valveInfo->safePosition != 0);

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(startValue));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
            qreal dacValue = m_mpi.GetDAC()->GetRawFromValue(current);
            task.value.push_back(dacValue);
        }

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(endValue));

        for (auto it = parameters.points.rbegin(); it != parameters.points.rend(); ++it) {
            qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
            qreal dacValue = m_mpi.GetDAC()->GetRawFromValue(current);
            task.value.push_back(dacValue);
        }

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(startValue));

        optionalTest->SetTask(task);
        dynamic_cast<StepTest *>(optionalTest)->Set_T_value(parameters.test_value);

        connect(optionalTest, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optionTest(Charts::Step);
        });

        connect(dynamic_cast<StepTest *>(optionalTest), &StepTest::GetPoints,
                this, &Program::GetPoints_stepTest,
                Qt::BlockingQueuedConnection);

        connect(dynamic_cast<StepTest *>(optionalTest), &StepTest::Results,
                this, &Program::StepTestResults);

        emit ClearPoints(Charts::Step);

        break;
    }
    default:
        emit StopTest();
        return;
    }

    QThread *threadTest = new QThread(this);
    optionalTest->moveToThread(threadTest);

    emit SetButtonInitEnabled(false);

    connect(threadTest, &QThread::started, optionalTest, &OptionTest::Process);
    connect(optionalTest, &OptionTest::EndTest, threadTest, &QThread::quit);

    connect(this, &Program::StopTest, optionalTest, &OptionTest::Stop);
    connect(threadTest, &QThread::finished, threadTest, &QThread::deleteLater);
    connect(threadTest, &QThread::finished, optionalTest, &OptionTest::deleteLater);

    connect(this, &Program::ReleaseBlock, optionalTest, &MainTest::ReleaseBlock);

    connect(optionalTest, &OptionTest::EndTest, this, &Program::EndTest);

    connect(optionalTest, &OptionTest::SetDAC, this, &Program::SetDAC);
    connect(optionalTest, &OptionTest::SetStartTime, this, &Program::SetTimeStart);

    m_testing = true;
    emit EnableSetTask(false);
    emit ClearPoints(Charts::CyclicSolenoid);
    threadTest->start();
}

void Program::GetPoints_stepTest(QVector<QVector<QPointF>> &points)
{
    emit GetPoints(points, Charts::Step);
}

void Program::UpdateCharts_optionTest(Charts chart)
{
    QVector<Point> points;

    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(chart, points);
}

void Program::TerminateTest()
{
    m_stopSetDac = true;
    m_dacEventloop->quit();
    emit StopTest();
}

void Program::button_set_position()
{
    m_stopSetDac = true;
    m_dacEventloop->quit();
}

void Program::button_DO(quint8 DO_num, bool state)
{
    m_mpi.SetDiscreteOutput(DO_num, state);
    emit SetButtonsDOChecked(m_mpi.GetDOStatus());
}

void Program::checkbox_autoInit(int state)
{
    m_waitForButton = (state == 0);
}
