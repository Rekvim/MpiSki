#include "Program.h"
#include <QPushButton>
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

    m_diPollTimer = new QTimer(this);
    m_diPollTimer->setInterval(50);
    connect(m_diPollTimer, &QTimer::timeout,
            this, &Program::pollDIForCyclic);
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

    if (sleep_ms > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout,
                m_dacEventloop, &QEventLoop::quit);
        timer.start(sleep_ms);

        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) { emit ReleaseBlock(); return; }

    if (wait_for_stop) {
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

qreal Program::currentPercent() // add
{
    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4.0) / 16.0) * 100.0;
    percent = qBound<qreal>(0.0, percent, 100.0);
    if (m_registry->GetValveInfo()->safePosition != 0)
        percent = 100.0 - percent;
    return percent;
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
    percent = qBound<qreal>(0.0, percent, 100.0);

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

    m_diPollTimer->stop();

    emit StopTest();
    emit SetTask(m_mpi.GetDAC()->GetValue());

    m_cyclicRunning = false;
}

void Program::SetDAC_real(qreal value)
{
    m_mpi.SetDAC_Real(value);
}


void Program::SetInitDOStates(const QVector<bool> &states)
{
    m_initDOStates      = states;
    m_savedInitDOStates = states;
}

void Program::button_init(SelectTests::PatternType pattern)
{
    // 0. Подготовка UI
    m_timerSensors->stop();
    m_timerDI->stop();
    emit SetButtonInitEnabled(false);
    emit SetSensorNumber(0);

    connectAndInitDevice();

    detectAndReportSensors();

    // Подготовка информации о клапане
    ValveInfo *valveInfo = m_registry->GetValveInfo();
    bool normalClosed = (valveInfo->safePosition == 0);

    // Измерение начального и конечного положения соленоида
    if (pattern == SelectTests::Pattern_B_SACVT ||
        pattern == SelectTests::Pattern_C_SACVT ||
        pattern ==  SelectTests::Pattern_C_SOVT) {

        if ((m_mpi.Version() & 0x40) != 0) {
            emit SetButtonsDOChecked(m_mpi.GetDOStatus());
            m_timerDI->start();
        }

        measureEndPositionShutoff(normalClosed);

        measureStartPositionShutoff(normalClosed);
    }

    // Измерение начального и конечного положения позиционера
    if (pattern == SelectTests::Pattern_B_CVT ||
        pattern == SelectTests::Pattern_C_CVT ||
        pattern ==  SelectTests::Pattern_B_SACVT||
        pattern ==  SelectTests::Pattern_C_SACVT) {

        measureStartPosition(normalClosed);
        measureEndPosition(normalClosed);
    }


    // Остановка дискретных выходов
    calculateAndApplyCoefficients();

    // Запись хода клапана (позиционера)
    if (pattern == SelectTests::Pattern_B_CVT ||
        pattern == SelectTests::Pattern_C_CVT ||
        pattern ==  SelectTests::Pattern_B_SACVT||
        pattern ==  SelectTests::Pattern_C_SACVT) {

        recordStrokeRange(normalClosed);
    }
    // Финализация и возврат UI в рабочее состояние
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

    // initialize
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
    ValveInfo *valveInfo = m_registry->GetValveInfo();
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
    percent = qBound<qreal>(0.0, percent, 100.0);
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

void Program::onStartStrokeTest()
{
    emit SetButtonInitEnabled(false);
    emit ClearPoints(Charts::Stroke);

    if (m_patternType == SelectTests::Pattern_C_SOVT
        || m_patternType == SelectTests::Pattern_C_SACVT
        || m_patternType == SelectTests::Pattern_B_SACVT)
    {
    }

    StrokeTest *strokeTest = new StrokeTest;
    QThread *threadTest = new QThread(this);
    strokeTest->moveToThread(threadTest);

    connect(threadTest, &QThread::started,
            strokeTest, &StrokeTest::Process);

    connect(strokeTest, &StrokeTest::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::StopTest,
            strokeTest, &StrokeTest::Stop);

    connect(threadTest, &QThread::finished,
            threadTest, &QThread::deleteLater);

    connect(threadTest, &QThread::finished,
            strokeTest, &StrokeTest::deleteLater);

    connect(this, &Program::ReleaseBlock,
            strokeTest, &MainTest::ReleaseBlock);

    connect(strokeTest, &StrokeTest::EndTest,
            this, &Program::EndTest);

    connect(strokeTest, &StrokeTest::UpdateGraph,
            this, &Program::UpdateCharts_strokeTest);

    connect(strokeTest, &StrokeTest::SetDAC,
            this, &Program::SetDAC);

    connect(strokeTest, &StrokeTest::SetStartTime,
            this, &Program::SetTimeStart);

    connect(strokeTest, &StrokeTest::Results,
            this, &Program::StrokeTestResults);

    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}


void Program::StrokeTestResults(quint64 forwardTime, quint64 backwardTime)
{

    m_telemetryStore.strokeTestRecord.timeForwardMs = forwardTime;
    m_telemetryStore.strokeTestRecord.timeBackwardMs = backwardTime;

    emit TelemetryUpdated(m_telemetryStore);
}

void Program::UpdateCharts_strokeTest()
{
    QVector<Point> points;

    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qBound<qreal>(0.0, percent, 100.0);

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

    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qBound<qreal>(0.0, percent, 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100.0 - percent;
    }

    quint64 t = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    QVector<Point> pts;
    pts.push_back({0, qreal(t), percent});
    pts.push_back({1, qreal(t), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::CyclicSolenoid, pts);
}

void Program::pollDIForCyclic()
{
    if (!m_cyclicRunning) return;
    quint8 di = m_mpi.GetDIStatus();
    if (di == m_lastDI) return;

    qreal t = QDateTime::currentMSecsSinceEpoch() - m_cyclicStartTs;
    QVector<Point> pts;

    // Извлекаем старый/новый статус каждого бит-канала
    bool lastClosed = (m_lastDI & 0x01);
    bool lastOpen = (m_lastDI & 0x02);
    bool nowClosed = (di & 0x01);
    bool nowOpen = (di & 0x02);

    // Кв закрыто: начало закрытой фазы
    if ( nowClosed && !lastClosed ) {
        ++m_telemetryStore.cyclicTestRecord.switch3to0Count;
        pts.push_back({ 2, t, 0.0 });
    }
    // Кв закрыто: конец закрытой фазы
    if ( !nowClosed && lastClosed ) {
        pts.push_back({ 2, t, 0.0 });
    }
    // Кв открыто: начало открытой фазы
    if ( nowOpen && !lastOpen ) {
        ++m_telemetryStore.cyclicTestRecord.switch0to3Count;
        pts.push_back({ 3, t, 100.0 });
    }
    // Кв открыто: конец открытой фазы
    if ( !nowOpen && lastOpen ) {
        pts.push_back({ 3, t, 100.0 });
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

QVector<quint16> Program::makeRawValues(const QString &seq, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.count('-') + 1);

    for (const QString &part : seq.split('-', Qt::SkipEmptyParts)) {
        int pct = part.trimmed().toInt();
        qreal current = 16.0 * (normalOpen ? 100 - pct : pct) / 100.0 + 4.0;
        raw.push_back(m_mpi.GetDAC()->GetRawFromValue(current));
    }
    return raw;
}

void Program::CyclicSolenoidTestStart(const CyclicTestSettings::TestParameters &p)
{
    using TP = CyclicTestSettings::TestParameters;

    m_cyclicRunning = true;

    // Валидация входных параметров
    bool ok = true;
    if (p.testType == TP::Regulatory
        || p.testType == TP::Combined) {
        ok &= !p.regulatory_sequence.isEmpty()
            &&  p.regulatory_delaySec > 0
            &&  p.regulatory_holdTimeSec >= 0
            &&  p.regulatory_numCycles > 0;
    }
    if (p.testType == TP::Shutoff
        || p.testType == TP::Combined) {
        ok &= !p.shutoff_sequence.isEmpty()
            &&  p.shutoff_delaySec > 0
            &&  p.shutoff_holdTimeSec >= 0
            &&  p.shutoff_numCycles > 0;
    }
    if (!ok) {
        emit StopTest();
        return;
    }

    // Парсим процентные значения для Regulatory
    QStringList partsReg = p.regulatory_sequence.split('-', Qt::SkipEmptyParts);
    QVector<int> valuesReg;
    valuesReg.reserve(partsReg.size());
    for (const QString &s : partsReg) {
        bool convOk = false;
        int v = s.trimmed().toInt(&convOk);
        if (convOk) valuesReg.append(v);
    }

    // Инициализация телеметрии
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

    m_diPollTimer->start();

    // процент → ток [4…20] → raw DAC
    ValveInfo *vi = m_registry->GetValveInfo();
    bool normalOpen = (vi->safePosition != 0);

    QVector<quint16> rawReg = makeRawValues(p.regulatory_sequence, normalOpen);
    QVector<quint16> rawOff;
    if (!p.shutoff_sequence.isEmpty()) {
        rawOff = makeRawValues(p.shutoff_sequence, normalOpen);
    }

    // Формируем параметры для теста
    TP params = p;
    params.rawRegValues = std::move(rawReg);
    params.rawOffValues = std::move(rawOff);

    emit SetButtonInitEnabled(false);

    // Создаём и настраиваем CyclicTestSolenoid
    auto *cyclicTestSolenoid = new CyclicTestSolenoid;
    cyclicTestSolenoid->SetParameters(params);

    QThread *threadTest = new QThread;
    cyclicTestSolenoid->moveToThread(threadTest);

    // Подключаем все сигналы и слоты
    connect(threadTest, &QThread::started,
            cyclicTestSolenoid, &CyclicTestSolenoid::Process);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::StopTest,
            cyclicTestSolenoid, &CyclicTestSolenoid::Stop);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::EndTest,
            this, &Program::EndTest);

    connect(threadTest, &QThread::finished,
            threadTest, &QObject::deleteLater);

    connect(threadTest, &QThread::finished,
            cyclicTestSolenoid, &QObject::deleteLater);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::UpdateGraph,
            this, &Program::UpdateCharts_CyclicSolenoid);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::SetDAC,
            this, &Program::SetDAC);

    if (p.testType == TP::Regulatory
        || p.testType == TP::Combined) {

        connect(cyclicTestSolenoid, &CyclicTestSolenoid::RegulatoryMeasurement,
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
            }
        );
    }

    if (p.testType == TP::Shutoff || p.testType == TP::Combined) {
        connect(cyclicTestSolenoid, &CyclicTestSolenoid::SetMultipleDO,
                this, &Program::SetMultipleDO);

        connect(cyclicTestSolenoid, &CyclicTestSolenoid::SetDO,
                this, &Program::button_DO);

        connect(cyclicTestSolenoid, &CyclicTestSolenoid::DOCounts,
                this, &Program::onDOCounts);
    }

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::ClearGraph,
            this, [&] { emit ClearPoints(Charts::CyclicSolenoid); });

    connect(this, &Program::ReleaseBlock,
            cyclicTestSolenoid, &CyclicTestSolenoid::ReleaseBlock);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::SetStartTime,
            this, &Program::SetTimeStart);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::testResults,
            this, &Program::SolenoidResults);

    connect(cyclicTestSolenoid, &CyclicTestSolenoid::CycleCompleted,
            this, &Program::CyclicCycleCompleted);

    // Финальная подготовка UI и старт
    emit ClearPoints(Charts::CyclicSolenoid);
    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
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

    connect(threadTest, &QThread::started,
            optionalTest, &OptionTest::Process);

    connect(optionalTest, &OptionTest::EndTest,
            threadTest, &QThread::quit);

    connect(this, &Program::StopTest,
            optionalTest, &OptionTest::Stop);

    connect(threadTest, &QThread::finished,
            threadTest, &QThread::deleteLater);

    connect(threadTest, &QThread::finished,
            optionalTest, &OptionTest::deleteLater);

    connect(this, &Program::ReleaseBlock,
            optionalTest, &MainTest::ReleaseBlock);

    connect(optionalTest, &OptionTest::EndTest,
            this, &Program::EndTest);

    connect(optionalTest, &OptionTest::SetDAC,
            this, &Program::SetDAC);

    connect(optionalTest, &OptionTest::SetStartTime,
            this, &Program::SetTimeStart);

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
    percent = qBound<qreal>(0.0, percent, 100.0);

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
    if (!m_isInitialized) {
        if ((int)m_initDOStates.size() < 4)
            m_initDOStates.resize(4);

        m_initDOStates[DO_num] = state;

        // сразу отражаем в UI
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
