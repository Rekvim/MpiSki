#include "Program.h"
#include <QPushButton>
// #include "./Src/Tests/CyclicTestPositioner.h"
#include "./Src/Tests/CyclicTests.h"
#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Src/Tests/MainTest.h"

Program::Program(Registry& registry, QObject* parent)
    : QObject(parent)
    , m_registry(registry)
{
    // тут
    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);

    m_dacEventloop = new QEventLoop(this);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::updateSensors);

    m_testing = false;

    m_timerDI = new QTimer(this);
    m_timerDI->setInterval(1000);
    connect(m_timerDI, &QTimer::timeout, this, [&]() {
        quint8 DI = m_mpi.GetDIStatus();
        emit SetCheckboxDIChecked(DI);
    });

    emit errorOccured("Не удалось установить связь с устройством");
}

void Program::SetDAC(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    m_stopSetDac = false;

    if (m_mpi.SensorCount() == 0) {
        emit ReleaseBlock();
        return;
    }

    m_mpi.SetDAC_Raw(dac);
    if (waitForStart) {
        QTimer timer;
        timer.setInterval(50);
        QList<quint16> lineSensor;
        connect(&timer, &QTimer::timeout, this, [&](){
            lineSensor.push_back(m_mpi[0]->GetRawValue());
            if (qAbs(lineSensor.first() - lineSensor.last()) > 10) {
                timer.stop();
                m_dacEventloop->quit();
            }
            if (lineSensor.size() > 50)
                lineSensor.pop_front();
        });

        timer.start();
        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) { emit ReleaseBlock(); return; }

    if (sleepMs > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout,
                m_dacEventloop, &QEventLoop::quit);
        timer.start(sleepMs);

        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) { emit ReleaseBlock(); return; }

    if (waitForStop) {
        QTimer timer;
        timer.setInterval(50);
        QList<quint16> lineSensor;
        connect(&timer, &QTimer::timeout, this, [&](){
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

qreal Program::calculatingK() {
    ValveInfo *valveInfo = m_registry.GetValveInfo();
    return 5 * M_PI * valveInfo->driveDiameter * valveInfo->driveDiameter / 4;
}

qreal Program::currentPercent() const
{
    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4.0) / 16.0) * 100.0;
    percent = qBound<qreal>(0.0, percent, 100.0);
    if (m_registry.GetValveInfo()->safePosition != 0)
        percent = 100.0 - percent;
    return percent;
}

void Program::updateSensors()
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
    qreal percent = currentPercent();

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_initTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::Trend, points);
}

void Program::endTest()
{
    m_testing = false;
    emit EnableSetTask(true);
    emit SetButtonInitEnabled(true);

    emit stopTheTest();
    emit SetTask(m_mpi.GetDAC()->GetValue());

    m_cyclicRunning = false;
}

void Program::SetDAC_real(qreal value)
{
    m_mpi.SetDAC_Real(value);
}


void Program::SetInitDOStates(const QVector<bool> &states)
{
    m_initDOStates = states;
    m_savedInitDOStates = states;
}

void Program::initialization()
{
    m_timerSensors->stop();
    m_timerDI->stop();
    emit SetButtonInitEnabled(false);
    emit SetSensorNumber(0);

    connectAndInitDevice();

    detectAndReportSensors();

    ValveInfo *valveInfo = m_registry.GetValveInfo();
    bool normalClosed = (valveInfo->safePosition == 0);

    // Измерение начального и конечного положения соленоида
    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        if ((m_mpi.Version() & 0x40) != 0) {
            emit SetButtonsDOChecked(m_mpi.GetDOStatus());
            m_timerDI->start();
        }

        measureEndPositionShutoff(normalClosed);
        measureStartPositionShutoff(normalClosed);
    }

    // Измерение начального и конечного положения позиционера
    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT) {

        measureStartPosition(normalClosed);
        measureEndPosition(normalClosed);
    }

    // Запись хода клапана (позиционера)
    // if (m_patternType == SelectTests::Pattern_B_SACVT||
    //     m_patternType == SelectTests::Pattern_C_SACVT) {

    //     recordStrokeRange(normalClosed);
    // }

    calculateAndApplyCoefficients();

    // Запись хода клапана (позиционера)
    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT ||
        m_patternType == SelectTests::Pattern_B_SACVT||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        recordStrokeRange(normalClosed);
    }

    finalizeInitialization();
}

// Подключение и инициализация устройства
void Program::connectAndInitDevice()
{
    auto &ts = m_telemetryStore;
    bool ok;

    ok = m_mpi.Connect();
    ts.init.deviceStatusText  = ok ? QString("Успешное подключение к порту %1").arg(m_mpi.PortName())
                                  : "Ошибка подключения";
    ts.init.deviceStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit TelemetryUpdated(ts);

    ok = m_mpi.Initialize();
    ts.init.initStatusText  = ok ? "Успешная инициализация"
                                : "Ошибка инициализации";
    ts.init.initStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit TelemetryUpdated(ts);

    m_isInitialized = ok;
}

// Обнаружение и отчёт по датчикам
void Program::detectAndReportSensors()
{
    auto &ts = m_telemetryStore;
    int cnt = m_mpi.SensorCount();

    if (cnt == 0) {
        ts.init.connectedSensorsText = "Датчики не обнаружены";
        ts.init.connectedSensorsColor = Qt::red;
    }
    else if (cnt == 1) {
        ts.init.connectedSensorsText = "Обнаружен 1 датчик";
        ts.init.connectedSensorsColor = Qt::darkYellow;
    }
    else {
        ts.init.connectedSensorsText = QString("Обнаружено %1 датчика").arg(cnt);
        ts.init.connectedSensorsColor = Qt::darkGreen;
    }

    emit TelemetryUpdated(ts);
}

void Program::waitForDacCycle()
{
    QTimer timer(this);
    connect(&timer, &QTimer::timeout, this, [&] {
        if (!m_waitForButton || m_stopSetDac)
            m_dacEventloop->quit();
    });
    timer.start(50);
    m_dacEventloop->exec();
    timer.stop();
}

void Program::measureStartPosition(bool normalClosed)
{
    auto &ts = m_telemetryStore;
    ts.init.startingPositionText  = "Измерение";
    ts.init.startingPositionColor = Qt::darkYellow;
    emit TelemetryUpdated(ts);

    SetDAC(0, 10000, true);
    waitForDacCycle();

    if (normalClosed) m_mpi[0]->SetMin();
    else m_mpi[0]->SetMax();

    ts.init.startingPositionText  = m_mpi[0]->GetFormatedValue();
    ts.init.startingPositionColor = Qt::darkGreen;
    emit TelemetryUpdated(ts);
}

void Program::measureStartPositionShutoff(bool normalClosed)
{
    auto &ts = m_telemetryStore;

    ts.init.startingPositionText  = "Измерение";
    ts.init.startingPositionColor = Qt::darkYellow;
    emit TelemetryUpdated(ts);

    for (int i = 0; i < m_savedInitDOStates.size(); ++i) {
        if (m_savedInitDOStates[i]) {
            m_initDOStates[i] = false;
            m_mpi.SetDiscreteOutput(i, false);
        }
    }
    emit SetButtonsDOChecked(m_mpi.GetDOStatus());

    SetDAC(0, 1000, true);
    waitForDacCycle();

    if (normalClosed) m_mpi[0]->SetMin();
    else m_mpi[0]->SetMax();

    ts.init.startingPositionText  = m_mpi[0]->GetFormatedValue();
    ts.init.startingPositionColor = Qt::darkGreen;
    emit TelemetryUpdated(ts);
}


void Program::measureEndPosition(bool normalClosed)
{
    auto &ts = m_telemetryStore;
    ts.init.finalPositionText  = "Измерение";
    ts.init.finalPositionColor = Qt::darkYellow;
    emit TelemetryUpdated(ts);

    SetDAC(0xFFFF, 10000, true);
    waitForDacCycle();

    if (normalClosed) m_mpi[0]->SetMax();
    else m_mpi[0]->SetMin();

    ts.init.finalPositionText  = m_mpi[0]->GetFormatedValue();
    ts.init.finalPositionColor = Qt::darkGreen;
    emit TelemetryUpdated(ts);
}

void Program::measureEndPositionShutoff(bool normalClosed)
{
    auto &s = m_telemetryStore;

    s.init.finalPositionText  = "Измерение";
    s.init.finalPositionColor = Qt::darkYellow;
    emit TelemetryUpdated(s);

    for (int i = 0; i < m_savedInitDOStates.size(); ++i) {
        if (m_savedInitDOStates[i]) {
            m_initDOStates[i] = true;
            m_mpi.SetDiscreteOutput(i, true);
        }
    }
    emit SetButtonsDOChecked(m_mpi.GetDOStatus());

    SetDAC(0xFFFF, 1000, true);
    waitForDacCycle();

    if (normalClosed) m_mpi[0]->SetMax();
    else m_mpi[0]->SetMin();

    s.init.finalPositionText  = m_mpi[0]->GetFormatedValue();
    s.init.finalPositionColor = Qt::darkGreen;
    emit TelemetryUpdated(s);
}

void Program::calculateAndApplyCoefficients()
{
    ValveInfo *valveInfo = m_registry.GetValveInfo();
    qreal coeff = 1.0;

    if (valveInfo->strokeMovement != 0) {
        coeff = qRadiansToDegrees(2.0 / valveInfo->diameterPulley);
        m_mpi[0]->SetUnit("°");
    }

    m_mpi[0]->CorrectCoefficients(coeff);
}

void Program::recordStrokeRange(bool normalClosed)
{
    auto &s = m_telemetryStore;

    if (normalClosed) {
        s.valveStrokeRecord.range = m_mpi[0]->GetFormatedValue();
        s.valveStrokeRecord.real = m_mpi[0]->GetValue();
        SetDAC(0);
    } else {
        SetDAC(0, 10000, true);
        s.valveStrokeRecord.range = m_mpi[0]->GetFormatedValue();
        s.valveStrokeRecord.real = m_mpi[0]->GetValue();
    }

    emit TelemetryUpdated(s);
    emit SetTask(m_mpi.GetDAC()->GetValue());
}

void Program::finalizeInitialization()
{
    emit ClearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    emit SetSensorNumber(m_mpi.SensorCount());
    emit SetButtonInitEnabled(true);
    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}

void Program::runningMainTest()
{
    MainTestSettings::TestParameters parameters;
    emit getParameters_mainTest(parameters);

    if (parameters.delay == 0) {
        emit stopTheTest();
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

    connect(this, &Program::stopTheTest,
            mainTest, &MainTest::StoppingTheTest);

    connect(threadTest, &QThread::finished,
            threadTest, &QThread::deleteLater);

    connect(threadTest, &QThread::finished,
            mainTest, &MainTest::deleteLater);

    connect(mainTest, &MainTest::EndTest,
            this, &Program::endTest);

    connect(mainTest, &MainTest::EndTest,
            this, &Program::MainTestFinished);

    connect(mainTest, &MainTest::UpdateGraph,
            this, &Program::updateCharts_mainTest);

    connect(mainTest, &MainTest::SetDAC,
            this, &Program::SetDAC);

    connect(mainTest, &MainTest::DublSeries,
            this, [&] { emit DublSeries(); });

    connect(mainTest, &MainTest::GetPoints,
            this, &Program::receivedPoints_mainTest,
            Qt::BlockingQueuedConnection);

    connect(mainTest, &MainTest::AddRegression,
            this, &Program::addRegression);

    connect(mainTest, &MainTest::AddFriction,
            this, &Program::addFriction);

    connect(this, &Program::ReleaseBlock,
            mainTest, &MainTest::ReleaseBlock);

    connect(mainTest, &MainTest::Results,
            this, &Program::results_mainTest);

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

void Program::receivedPoints_mainTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_mainTest(points, Charts::Task);
}

void Program::results_mainTest(MainTest::TestResults results)
{
    qreal k = calculatingK();

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

void Program::updateCharts_mainTest()
{
    QVector<Point> points;

    qreal percent = currentPercent();

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

void Program::addFriction(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;

    qreal k = calculatingK();

    for (QPointF point : points) {
        chartPoints.push_back({0, point.x(), point.y() * k});
    }
    emit AddPoints(Charts::Friction, chartPoints);
}

void Program::addRegression(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;
    for (QPointF point : points) {
        chartPoints.push_back({1, point.x(), point.y()});
    }
    emit AddPoints(Charts::Pressure, chartPoints);

    //emit SetVisible(Charts::Pressure, 1, true);
    emit SetRegressionEnable(true);
}

void Program::runningStrokeTest()
{
    emit SetButtonInitEnabled(false);
    emit ClearPoints(Charts::Stroke);

    emit SetDOControlsEnabled(false);

    for (int i = 0; i < 4; ++i) {
        m_enabledDO[i] = (i < m_savedInitDOStates.size())
        ? m_savedInitDOStates[i]
        : false;
    }
    SetMultipleDO(m_savedInitDOStates);

    StrokeTest *strokeTest = new StrokeTest;

    strokeTest->SetPatternType(m_patternType);

    QThread *threadTest = new QThread(this);
    strokeTest->moveToThread(threadTest);

    connect(threadTest, &QThread::started,
            strokeTest, &StrokeTest::Process);

    connect(strokeTest, &StrokeTest::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::stopTheTest,
            strokeTest, &StrokeTest::StoppingTheTest);

    connect(threadTest, &QThread::finished,
            threadTest, &QThread::deleteLater);

    connect(threadTest, &QThread::finished,
            strokeTest, &StrokeTest::deleteLater);

    connect(this, &Program::ReleaseBlock,
            strokeTest, &MainTest::ReleaseBlock);

    connect(strokeTest, &StrokeTest::EndTest, this, [&]{
        emit SetDOControlsEnabled(true);
        endTest();
    });

    connect(strokeTest, &StrokeTest::UpdateGraph,
            this, &Program::updateCharts_strokeTest);

    connect(strokeTest, &StrokeTest::SetDAC,
            this, &Program::SetDAC);

    connect(strokeTest, &StrokeTest::SetMultipleDO,
            this, &Program::SetMultipleDO);

    connect(strokeTest, &StrokeTest::SetStartTime,
            this, &Program::SetTimeStart);

    connect(strokeTest, &StrokeTest::Results,
            this, &Program::results_strokeTest);

    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}


void Program::results_strokeTest(const quint64 forwardTime, const  quint64 backwardTime)
{

    m_telemetryStore.strokeTestRecord.timeForwardMs = forwardTime;
    m_telemetryStore.strokeTestRecord.timeBackwardMs = backwardTime;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::updateCharts_strokeTest()
{
    QVector<Point> points;

    qreal percent = currentPercent();

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::Stroke, points);
}

// void Program::SetMultipleDO(const QVector<bool>& states)
// {
//     // quint8 mask = 0;
//     for (int d = 0; d < states.size(); ++d) {
//         button_DO(d, states[d]); // меняет DO через SetDiscreteOutput
//         // if (states[d]) mask |= (1 << d);
//     }
//     // emit SetButtonsDOChecked(mask);
// }

// void Program::SetMultipleDO(const QVector<bool>& states)
// {
//     quint8 mask = 0;
//     for (int d = 0; d < states.size(); ++d) {
//         m_mpi.SetDiscreteOutput(d, states[d]);
//         if (states[d]) mask |= (1 << d);
//     }
//     emit SetButtonsDOChecked(mask);
// }

void Program::SetMultipleDO(const QVector<bool>& states)
{
    quint8 mask = 0;

    for (int i = 0; i < states.size() && i < int(m_enabledDO.size()); ++i) {
        if (!m_enabledDO[i]) continue;

        m_mpi.SetDiscreteOutput(i, states[i]);
        if (states[i])
            mask |= (1 << i);
    }

    emit SetButtonsDOChecked(mask);
}

QVector<quint16> Program::makeRawValues(const QVector<quint16> &seq, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.size());

    for (quint16 pct : seq) {
        qreal current = 16.0 * (normalOpen ? (100 - pct) : pct) / 100.0 + 4.0;
        raw.push_back(m_mpi.GetDAC()->GetRawFromValue(current));
    }
    return raw;
}

void Program::receivedPoints_cyclicTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_cyclicTest(points, Charts::Cyclic);
}

void Program::runningCyclicTest(const CyclicTestSettings::TestParameters &p)
{
    using TP = CyclicTestSettings::TestParameters;
    m_cyclicRunning = true;

    bool ok = true;
    if (p.testType == TP::Regulatory || p.testType == TP::Combined) {
        ok &= !p.regSeqValues.isEmpty()
        &&  p.regulatory_delaySec > 0
            &&  p.regulatory_holdTimeSec >= 0
            &&  p.regulatory_numCycles > 0;
    }
    if (p.testType == TP::Shutoff || p.testType == TP::Combined) {
        ok &= !p.offSeqValues.isEmpty()
        &&  p.shutoff_delaySec > 0
            &&  p.shutoff_holdTimeSec >= 0
            &&  p.shutoff_numCycles > 0;
    }
    if (!ok) { emit stopTheTest(); return; }

    const QVector<quint16> &valuesReg = p.regSeqValues;

    int recordCount = valuesReg.size();
    m_telemetryStore.cyclicTestRecord.ranges.clear();
    m_telemetryStore.cyclicTestRecord.ranges.resize(recordCount);

    for (int i = 0; i < recordCount; ++i) {
        auto &rec = m_telemetryStore.cyclicTestRecord.ranges[i];
        rec.rangePercent = valuesReg[i];
        rec.maxForwardValue = 0;
        rec.maxForwardCycle = 0;
        rec.maxReverseValue = 0;
        rec.maxReverseCycle = 0;
    }

    m_telemetryStore.cyclicTestRecord.switch3to0Count = 0;
    m_telemetryStore.cyclicTestRecord.switch0to3Count = 0;

    m_lastDI = m_mpi.GetDIStatus();
    m_cyclicStartTs= QDateTime::currentMSecsSinceEpoch();

    m_enabledDO = p.shutoff_DO;

    // процент -> ток
    ValveInfo *vi = m_registry.GetValveInfo();
    bool normalOpen = (vi->safePosition != 0);

    QVector<quint16> rawReg = makeRawValues(p.regSeqValues, normalOpen);
    QVector<quint16> rawOff = makeRawValues(p.offSeqValues, normalOpen);

    TP params = p;
    params.rawRegValues = std::move(rawReg);
    params.rawOffValues = std::move(rawOff);

    emit SetButtonInitEnabled(false);
    emit SetDOControlsEnabled(false);

    auto *cyclicTests = new CyclicTests;
    cyclicTests->SetParameters(params);

    QThread *threadTest = new QThread(this);
    cyclicTests->moveToThread(threadTest);

    connect(threadTest, &QThread::started,
            cyclicTests, &CyclicTests::Process);

    connect(cyclicTests, &CyclicTests::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::stopTheTest,
            cyclicTests, &CyclicTests::StoppingTheTest);

    connect(threadTest, &QThread::finished,
            threadTest, &QObject::deleteLater);

    connect(threadTest, &QThread::finished,
            cyclicTests, &QObject::deleteLater);

    connect(cyclicTests, &CyclicTests::EndTest, this, [this] {
        emit SetDOControlsEnabled(true);
        endTest();
    });

    connect(cyclicTests, &CyclicTests::UpdateGraph,
            this, &Program::updateCharts_CyclicTest);

    connect(cyclicTests, &CyclicTests::SetDAC,
            this, &Program::SetDAC);

    connect(cyclicTests, &CyclicTests::GetPoints,
            this, &Program::receivedPoints_cyclicTest,
            Qt::BlockingQueuedConnection);

    // if (p.testType == TP::Regulatory
    //     || p.testType == TP::Combined) {

    // }

    if (p.testType == TP::Shutoff || p.testType == TP::Combined) {

        connect(cyclicTests, &CyclicTests::SetMultipleDO,
                this, &Program::SetMultipleDO);
    }

    connect(this, &Program::ReleaseBlock,
            cyclicTests, &CyclicTests::ReleaseBlock);

    connect(cyclicTests, &CyclicTests::SetStartTime,
            this, &Program::SetTimeStart);

    connect(cyclicTests, &CyclicTests::Results,
            this, &Program::results_cyclicTests);

    connect(cyclicTests, &CyclicTests::CycleCompleted,
            this, &Program::CyclicCycleCompleted);

    connect(cyclicTests, &CyclicTests::ClearGraph,
            this, [&] { emit ClearPoints(Charts::Cyclic); });

    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}

void Program::results_cyclicTests(const CyclicTests::TestResults& r)
{
    auto &dst = m_telemetryStore.cyclicTestRecord;
    dst.sequence = r.sequence;
    dst.cycles = r.cycles;
    dst.totalTimeSec = r.totalTimeSec;

    dst.ranges.resize(r.ranges.size());
    for (int i = 0; i < r.ranges.size(); ++i) {
        const auto &src = r.ranges[i];
        auto &d = dst.ranges[i];
        d.rangePercent = src.rangePercent;
        d.maxForwardValue = src.maxForwardValue;
        d.maxForwardCycle = src.maxForwardCycle;
        d.maxReverseValue = src.maxReverseValue;
        d.maxReverseCycle = src.maxReverseCycle;
    }

    dst.doOnCounts = r.doOnCounts;
    dst.doOffCounts = r.doOffCounts;
    dst.switch3to0Count = r.switch3to0Count / 2;
    dst.switch0to3Count = r.switch0to3Count / 2;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::updateCharts_CyclicTest()
{
    if (!m_cyclicRunning) return;

    qreal percent = currentPercent();

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    QVector<Point> points;
    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});
    emit AddPoints(Charts::Cyclic, points);

    quint8 di = m_mpi.GetDIStatus();
    if (di != m_lastDI) {
        QVector<Point> diPts;
        bool lastClosed = (m_lastDI & 0x01);
        bool lastOpen = (m_lastDI & 0x02);
        bool nowClosed = (di & 0x01);
        bool nowOpen = (di & 0x02);

        if (nowClosed && !lastClosed) {
            ++m_telemetryStore.cyclicTestRecord.switch3to0Count;
            diPts.push_back({2, qreal(time), 0.0});
        } if (!nowClosed && lastClosed) {
            diPts.push_back({2, qreal(time), 0.0});
        } if (nowOpen && !lastOpen) {
            ++m_telemetryStore.cyclicTestRecord.switch0to3Count;
            diPts.push_back({3, qreal(time), 100.0});
        } if (!nowOpen && lastOpen) {
            diPts.push_back({3, qreal(time), 100.0});
        }

        if (!diPts.isEmpty()) emit AddPoints(Charts::Cyclic, diPts);
        m_lastDI = di;
    }
}

void Program::runningOptionalTest(quint8 testNum)
{
    OptionTest::Task task;
    OptionTest *optionalTest;

    switch (testNum) {
    case 0: {
        optionalTest = new OptionTest;

        OtherTestSettings::TestParameters parameters;
        emit getParameters_responseTest(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit stopTheTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valveInfo = m_registry.GetValveInfo();

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
            updateCharts_optionTest(Charts::Response);
        });

        emit ClearPoints(Charts::Response);

        break;
    }
    case 1: {
        optionalTest = new OptionTest;
        OtherTestSettings::TestParameters parameters;
        emit getParameters_resolutionTest(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit stopTheTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valveInfo = m_registry.GetValveInfo();

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
            updateCharts_optionTest(Charts::Resolution);
        });

        emit ClearPoints(Charts::Resolution);

        break;
    }

    case 2: {
        optionalTest = new StepTest;
        StepTestSettings::TestParameters parameters;
        emit getParameters_stepTest(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit stopTheTest();
            return;
        }

        task.delay = parameters.delay;
        ValveInfo *valveInfo = m_registry.GetValveInfo();

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
            updateCharts_optionTest(Charts::Step);
        });

        connect(dynamic_cast<StepTest *>(optionalTest), &StepTest::GetPoints,
                this, &Program::receivedPoints_stepTest,
                Qt::BlockingQueuedConnection);

        connect(dynamic_cast<StepTest *>(optionalTest), &StepTest::Results,
                this, &Program::results_stepTest);

        emit ClearPoints(Charts::Step);

        break;
    }
    default:
        emit stopTheTest();
        return;
    }

    QThread *threadTest = new QThread(this);
    optionalTest->moveToThread(threadTest);

    emit SetButtonInitEnabled(false);

    connect(threadTest, &QThread::started,
            optionalTest, &OptionTest::Process);

    connect(optionalTest, &OptionTest::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::stopTheTest,
            optionalTest, &OptionTest::StoppingTheTest);

    connect(threadTest, &QThread::finished,
            threadTest, &QThread::deleteLater);

    connect(threadTest, &QThread::finished,
            optionalTest, &OptionTest::deleteLater);

    connect(this, &Program::ReleaseBlock,
            optionalTest, &MainTest::ReleaseBlock);

    connect(optionalTest, &OptionTest::EndTest,
            this, &Program::endTest);

    connect(optionalTest, &OptionTest::SetDAC,
            this, &Program::SetDAC);

    connect(optionalTest, &OptionTest::SetStartTime,
            this, &Program::SetTimeStart);

    m_testing = true;
    emit EnableSetTask(false);
    emit ClearPoints(Charts::Cyclic);
    threadTest->start();
}

void Program::receivedPoints_stepTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_optionTest(points, Charts::Step);
}

void Program::results_stepTest(QVector<StepTest::TestResult> results, quint32 T_value)
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

void Program::updateCharts_optionTest(Charts chart)
{
    QVector<Point> points;

    qreal percent = currentPercent();

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(chart, points);
}

void Program::button_set_position()
{
    m_stopSetDac = true;
    m_dacEventloop->quit();
}

void Program::button_DO(quint8 DO_num, bool state)
{
    if (!m_isInitialized) {
        if ((int)m_initDOStates.size() < 4)
            m_initDOStates.resize(4);

        m_initDOStates[DO_num] = state;

        quint8 mask = 0;
        for (int i = 0; i < m_initDOStates.size(); ++i)
            if (m_initDOStates[i]) mask |= (1 << i);

        emit SetButtonsDOChecked(mask);
        return;
    }

    m_mpi.SetDiscreteOutput(DO_num, state);
    emit SetButtonsDOChecked(m_mpi.GetDOStatus());
}

void Program::checkbox_autoInit(int state)
{
    m_waitForButton = (state == 0);
}

void Program::terminateTest()
{
    m_stopSetDac = true;
    m_dacEventloop->quit();
    emit stopTheTest();
}
