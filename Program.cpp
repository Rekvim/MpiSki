#include "Program.h"

#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Src/Tests/MainTest.h"
#include "./Src/Runners/MainTestRunner.h"
#include "./Src/Runners/StepTestRunner.h"
#include "./Src/Runners/StrokeTestRunner.h"
#include "./Src/Runners/OptionResponseRunner.h"
#include "./Src/Runners/OptionResolutionRunner.h"
#include "./Src/Runners/CyclicRegulatoryRunner.h"
#include "./Src/Runners/CyclicShutoffRunner.h"

Program::Program(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<RealtimeState>("RealtimeState");

    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);

    m_dacEventLoop = new QEventLoop(this);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::updateSensors);

    m_isTestRunning = false;

    m_timerDI = new QTimer(this);
    m_timerDI->setInterval(1000);
    connect(m_timerDI, &QTimer::timeout, this, [&]() {
        quint8 DI = m_mpi.GetDIStatus();
        emit setDiCheckboxesChecked(DI);
    });

    // connect(&m_mpi, &MPI::errorOccured,
    //         this, &Program::errorOccured,
    //         Qt::QueuedConnection);
}

void Program::setRegistry(Registry *registry)
{
    m_registry = registry;
}

void Program::setDacRaw(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    m_isDacStopRequested = false;

    if (m_mpi.SensorCount() == 0) {
        emit releaseBlock();
        return;
    }

    m_mpi.SetDAC_Raw(dac);

    if (waitForStart) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->rawValue());
            if (qAbs(lineSensor.first() - lineSensor.last()) > 10) {
                timer.stop();
                m_dacEventLoop->quit();
            }
            if (lineSensor.size() > 50) {
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventLoop->exec();
        timer.stop();
    }

    if (m_isDacStopRequested) {
        emit releaseBlock();
        return;
    }

    if (sleepMs > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout, m_dacEventLoop, &QEventLoop::quit);
        timer.start(sleepMs);
        m_dacEventLoop->exec();
        timer.stop();
    }

    if (m_isDacStopRequested) {
        emit releaseBlock();
        return;
    }

    if (waitForStop) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->rawValue());
            if (lineSensor.size() == 50) {
                if (qAbs(lineSensor.first() - lineSensor.last()) < 10) {
                    timer.stop();
                    m_dacEventLoop->quit();
                }
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventLoop->exec();
        timer.stop();
    }

    emit releaseBlock();
}

void Program::setTimeStart()
{
    m_startTime = QDateTime::currentMSecsSinceEpoch();
}

void Program::updateSensors()
{
    for (quint8 i = 0; i < m_mpi.SensorCount(); ++i) {
        switch (i) {
        case 0:
            emit setText(TextObjects::LineEdit_linearSensor, m_mpi[i]->formattedValue());
            emit setText(TextObjects::LineEdit_linearSensorPercent, m_mpi[i]->formattedValue());
            break;
        case 1:
            emit setText(TextObjects::LineEdit_pressureSensor_1, m_mpi[i]->formattedValue());
            break;
        case 2:
            emit setText(TextObjects::LineEdit_pressureSensor_2, m_mpi[i]->formattedValue());
            break;
        case 3:
            emit setText(TextObjects::LineEdit_pressureSensor_3, m_mpi[i]->formattedValue());
            break;
        }
    }
    if (m_isTestRunning)

    emit setTask(m_mpi.GetDac()->value());

    Sensor *feedbackSensor = m_mpi.GetDac();
    QString fbValue = feedbackSensor->formattedValue();
    emit setText(TextObjects::LineEdit_feedback_4_20mA, fbValue);

    QVector<Point> points;
    qreal percent = calcPercent(m_mpi.GetDac()->value(),
                                m_registry->getValveInfo()->safePosition != 0);

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_initTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(Charts::Trend, points);
}


void Program::endTest()
{
    m_isTestRunning = false;

    emit setTaskControlsEnabled(true);
    emit setButtonInitEnabled(true);

    emit setTask(m_mpi.GetDac()->value());

    m_activeRunner.reset();
    m_isCyclicTestRunning = false;
    emit testFinished();
}

void Program::disposeActiveRunnerAsync() {
    m_activeRunner.reset();
}

void Program::setDAC_real(qreal value)
{
    m_mpi.SetDAC_Real(value);
}

void Program::setInitDOStates(const QVector<bool> &states)
{
    m_initialDoStates = states;
    m_savedInitialDoStates = states;
}

void Program::initialization()
{
    auto &ts = m_telemetryStore;

    ts.init.initStatusText = "";
    ts.init.connectedSensorsText = "";
    ts.init.deviceStatusText = "";
    ts.init.startingPositionText = "";
    ts.init.finalPositionText = "";

    m_timerSensors->stop();
    m_timerDI->stop();
    emit setButtonInitEnabled(false);
    emit setSensorNumber(0);

    if (!connectAndInitDevice()) {
        emit setButtonInitEnabled(true);
        return;
    }

    if (!detectAndReportSensors()) {
        emit setButtonInitEnabled(true);
        return;
    }

    ValveInfo *valveInfo = m_registry->getValveInfo();
    bool normalClosed = (valveInfo->safePosition == 0);

    // Измерение начального и конечного положения соленоида
    if (m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT ||
        m_patternType == SelectTests::Pattern_C_SOVT) {

        if ((m_mpi.Version() & 0x40) != 0) {
            emit setDoButtonsChecked(m_mpi.GetDOStatus());
            m_timerDI->start();
        } else {
            return;
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
bool Program::connectAndInitDevice()
{
    auto &ts = m_telemetryStore;
    bool ok = m_mpi.Connect();
    ts.init.deviceStatusText  = ok
                                   ? QString("Успешное подключение к порту %1").arg(m_mpi.PortName())
                                   : "Ошибка подключения";
    ts.init.deviceStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit telemetryUpdated(ts);

    if (!ok) {
        return false;
    }

    ok = m_mpi.Initialize();
    ts.init.initStatusText  = ok ? "Успешная инициализация" : "Ошибка инициализации";
    ts.init.initStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit telemetryUpdated(ts);

    m_isInitialized = ok;
    return ok;
}

// Обнаружение и отчёт по датчикам
bool Program::detectAndReportSensors()
{
    auto &ts = m_telemetryStore;
    int cnt = m_mpi.SensorCount();

    if (cnt == 0) {
        ts.init.connectedSensorsText = "Датчики не обнаружены";
        ts.init.connectedSensorsColor = Qt::red;
    }
    else if (cnt == 1) {
        ts.init.connectedSensorsText  = "Обнаружен 1 датчик";
        ts.init.connectedSensorsColor = Qt::darkYellow;
    }
    else {
        ts.init.connectedSensorsText = QString("Обнаружено %1 датчика").arg(cnt);
        ts.init.connectedSensorsColor = Qt::darkGreen;
    }

    emit telemetryUpdated(ts);

    return cnt > 0;
}

void Program::waitForDacCycle()
{
    QTimer timer(this);
    connect(&timer, &QTimer::timeout, this, [&] {
        if (!m_shouldWaitForButton || m_isDacStopRequested)
            m_dacEventLoop->quit();
    });
    timer.start(50);
    m_dacEventLoop->exec();
    timer.stop();
}

void Program::measureStartPosition(bool normalClosed)
{
    auto &ts = m_telemetryStore;
    ts.init.startingPositionText = "Измерение";
    ts.init.startingPositionColor = Qt::darkYellow;
    emit telemetryUpdated(ts);

    setDacRaw(0, 10000, true);
    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMin();
    else m_mpi[0]->captureMax();

    ts.init.startingPositionText  = m_mpi[0]->formattedValue();
    ts.init.startingPositionColor = Qt::darkGreen;
    emit telemetryUpdated(ts);
}

void Program::measureEndPosition(bool normalClosed)
{
    auto &ts = m_telemetryStore;
    ts.init.finalPositionText = "Измерение";
    ts.init.finalPositionColor = Qt::darkYellow;
    emit telemetryUpdated(ts);

    setDacRaw(0xFFFF, 10000, true);

    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMax();
    else m_mpi[0]->captureMin();

    ts.init.finalPositionText  = m_mpi[0]->formattedValue();
    ts.init.finalPositionColor = Qt::darkGreen;
    emit telemetryUpdated(ts);
}

void Program::measureStartPositionShutoff(bool normalClosed)
{
    auto &ts = m_telemetryStore;

    ts.init.startingPositionText  = "Измерение";
    ts.init.startingPositionColor = Qt::darkYellow;
    emit telemetryUpdated(ts);

    for (int i = 0; i < m_savedInitialDoStates.size(); ++i) {
        if (m_savedInitialDoStates[i]) {
            m_initialDoStates[i] = false;
            m_mpi.SetDiscreteOutput(i, false);
        }
    }
    emit setDoButtonsChecked(m_mpi.GetDOStatus());

    setDacRaw(0, 10000, true, true);

    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMin();
    else m_mpi[0]->captureMax();

    ts.init.startingPositionText = m_mpi[0]->formattedValue();
    ts.init.startingPositionColor = Qt::darkGreen;
    emit telemetryUpdated(ts);
}

void Program::measureEndPositionShutoff(bool normalClosed)
{
    auto &s = m_telemetryStore;

    s.init.finalPositionText  = "Измерение";
    s.init.finalPositionColor = Qt::darkYellow;
    emit telemetryUpdated(s);

    for (int i = 0; i < m_savedInitialDoStates.size(); ++i) {
        if (m_savedInitialDoStates[i]) {
            m_initialDoStates[i] = true;
            m_mpi.SetDiscreteOutput(i, true);
        }
    }
    emit setDoButtonsChecked(m_mpi.GetDOStatus());

    setDacRaw(65535.0, 1000, true);

    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMax();
    else m_mpi[0]->captureMin();

    s.init.finalPositionText  = m_mpi[0]->formattedValue();
    s.init.finalPositionColor = Qt::darkGreen;
    emit telemetryUpdated(s);
}

void Program::calculateAndApplyCoefficients()
{
    ValveInfo *valveInfo = m_registry->getValveInfo();
    qreal coeff = 1.0;

    if (valveInfo->strokeMovement != 0) {
        coeff = qRadiansToDegrees(2.0 / valveInfo->diameterPulley);
        m_mpi[0]->setUnit("°");
    }

    m_mpi[0]->correctCoefficients(coeff);
}

void Program::recordStrokeRange(bool normalClosed)
{
    auto &s = m_telemetryStore;

    if (normalClosed) {
        s.valveStrokeRecord.range = m_mpi[0]->formattedValue();
        s.valveStrokeRecord.real = m_mpi[0]->value();
        setDacRaw(0);
    } else {
        setDacRaw(0, 10000, true);
        s.valveStrokeRecord.range = m_mpi[0]->formattedValue();
        s.valveStrokeRecord.real = m_mpi[0]->value();
    }

    emit telemetryUpdated(s);
    emit setTask(m_mpi.GetDac()->value());
}

void Program::finalizeInitialization()
{
    emit clearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    emit setSensorNumber(m_mpi.SensorCount());
    emit setButtonInitEnabled(true);
    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}


void Program::startMainTest()
{
    auto runner = std::make_unique<MainTestRunner>(m_mpi, *m_registry, this);

    connect(runner.get(), &AbstractTestRunner::requestSetDAC,
            this, &Program::setDacRaw);

    connect(this, &Program::releaseBlock,
            runner.get(), &AbstractTestRunner::releaseBlock);

    connect(runner.get(), &MainTestRunner::getParameters_mainTest,
            this, &Program::forwardGetParameters_mainTest);

    connect(runner.get(), &AbstractTestRunner::totalTestTimeMs,
            this, &Program::totalTestTimeMs);

    connect(runner.get(), &AbstractTestRunner::endTest,
            this, &Program::endTest);

    connect(this, &Program::stopTheTest,
            runner.get(), &AbstractTestRunner::stop);

    emit setButtonInitEnabled(false);
    emit setTaskControlsEnabled(false);

    m_isTestRunning = true;

    m_activeRunner = std::move(runner);
    m_activeRunner->start();
}

void Program::receivedPoints_mainTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_mainTest(points, Charts::Task);
}

static bool inRange(double value, double lower, double upper)
{
    if (lower > upper)
        std::swap(lower, upper);
    return value >= lower && value <= upper;
}

static bool rangeOverlap(double valueLow, double valueHigh,
                         double limitLow, double limitHigh)
{
    if (limitLow > limitHigh)
        std::swap(limitLow, limitHigh);
    if (valueLow > valueHigh)
        std::swap(valueLow, valueHigh);

    return (valueLow >= limitLow && valueHigh <= limitHigh);
}

void Program::updateCrossingStatus()
{
    auto &ts = m_telemetryStore;
    const ValveInfo *valveInfo = m_registry->getValveInfo();
    const CrossingLimits &limits = valveInfo->crossingLimits;
    using State = CrossingStatus::State;

    if (limits.frictionEnabled) {
        ts.crossingStatus.frictionPercent =
            inRange(ts.mainTestRecord.frictionPercent,
                    limits.frictionCoefLowerLimit,
                    limits.frictionCoefUpperLimit)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.frictionPercent = State::Unknown;
    }

    if (limits.rangeEnabled) {
        ts.crossingStatus.range =
            inRange(ts.valveStrokeRecord.real,
                    0.0,
                    limits.rangeUpperLimit)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.range = State::Unknown;
    }

    if (limits.dynamicErrorEnabled) {
        ts.crossingStatus.dynamicError =
            inRange(ts.mainTestRecord.dynamicErrorReal,
                    0.0,
                    valveInfo->dinamicErrorRecomend)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.dynamicError = State::Unknown;
    }

    if (limits.springEnabled) {
        ts.crossingStatus.spring =
            rangeOverlap(ts.mainTestRecord.springLow,
                         ts.mainTestRecord.springHigh,
                         limits.springLowerLimit,
                         limits.springUpperLimit)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.spring = State::Unknown;
    }

    if (limits.linearCharacteristicEnabled) {
        ts.crossingStatus.linearCharacteristic = State::Ok;
    } else {
        ts.crossingStatus.linearCharacteristic = State::Unknown;
    }
}


void Program::results_mainTest(const MainTest::TestResults &results)
{
    ValveInfo *valveInfo = m_registry->getValveInfo();

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

    s.linearityError = results.linearityError;
    s.linearity = results.linearity;

    updateCrossingStatus();
    emit telemetryUpdated(m_telemetryStore);
}

void Program::updateCharts_mainTest()
{
    QVector<Point> points;

    qreal percent = calcPercent(m_mpi.GetDac()->value(),
                                m_registry->getValveInfo()->safePosition != 0);

    qreal task = m_mpi[0]->valueFromPercent(percent);
    qreal X = m_mpi.GetDac()->value();
    points.push_back({0, X, task});

    for (quint8 i = 0; i < m_mpi.SensorCount(); ++i) {
        points.push_back({static_cast<quint8>(i + 1), X, m_mpi[i]->value()});
    }

    emit addPoints(Charts::Task, points);

    points.clear();
    points.push_back({0, m_mpi[1]->value(), m_mpi[0]->value()});

    emit addPoints(Charts::Pressure, points);
}

void Program::addFriction(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;

    ValveInfo *valveInfo = m_registry->getValveInfo();

    qreal k = 5 * M_PI * valveInfo->driveDiameter * valveInfo->driveDiameter / 4;

    for (QPointF point : points) {
        chartPoints.push_back({0, point.x(), point.y() * k});
    }
    emit addPoints(Charts::Friction, chartPoints);
}

void Program::addRegression(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;
    for (QPointF point : points) {
        chartPoints.push_back({1, point.x(), point.y()});
    }
    emit addPoints(Charts::Pressure, chartPoints);

    emit setRegressionEnable(true);
}

void Program::startStrokeTest() {
    auto r = std::make_unique<StrokeTestRunner>(m_mpi, *m_registry, this);
    startRunner(std::move(r));
}

void Program::results_strokeTest(const quint64 forwardTime, const quint64 backwardTime)
{
    QString forwardText = QTime(0, 0).addMSecs(forwardTime).toString("mm:ss.zzz");
    QString backwardText = QTime(0, 0).addMSecs(backwardTime).toString("mm:ss.zzz");

    m_telemetryStore.strokeTestRecord.timeForwardMs = forwardText;
    m_telemetryStore.strokeTestRecord.timeBackwardMs = backwardText;

    emit telemetryUpdated(m_telemetryStore);
}

void Program::updateCharts_strokeTest()
{
    QVector<Point> points;

    qreal percent = calcPercent(m_mpi.GetDac()->value(),
                                m_registry->getValveInfo()->safePosition != 0);

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(Charts::Stroke, points);
}

void Program::updateCharts_CyclicTest(Charts chart)
{
    QVector<Point> points;

    qreal percent = calcPercent(m_mpi.GetDac()->value(),
                                m_registry->getValveInfo()->safePosition != 0);

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});
    emit addPoints(chart, points);


    // if (m_patternType == SelectTests::Pattern_C_SOVT  ||
    //     m_patternType == SelectTests::Pattern_B_SACVT ||
    //     m_patternType == SelectTests::Pattern_C_SACVT) {
    //     quint8 di = m_mpi.GetDIStatus();
    //     if (di != m_lastDiStatus) {
    //         QVector<Point> diPts;
    //         bool lastClosed = (m_lastDiStatus & 0x01);
    //         bool lastOpen = (m_lastDiStatus & 0x02);
    //         bool nowClosed = (di & 0x01);
    //         bool nowOpen = (di & 0x02);

    //         if (nowClosed && !lastClosed) {
    //             ++m_telemetryStore.cyclicTestRecord.switch3to0Count;
    //             diPts.push_back({2, qreal(time), 0.0});
    //         } if (!nowClosed && lastClosed) {
    //             diPts.push_back({2, qreal(time), 0.0});
    //         } if (nowOpen && !lastOpen) {
    //             ++m_telemetryStore.cyclicTestRecord.switch0to3Count;
    //             diPts.push_back({3, qreal(time), 100.0});
    //         } if (!nowOpen && lastOpen) {
    //             diPts.push_back({3, qreal(time), 100.0});
    //         }

    //         if (!diPts.isEmpty()) emit addPoints(chart, diPts);
    //         m_lastDiStatus = di;
    //     }
    // }
}

QVector<quint16> Program::makeRawValues(const QVector<quint16> &seq, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.size());

    for (quint16 pct : seq) {
        qreal current = 16.0 * (normalOpen ? 100 - pct : pct) / 100.0 + 4.0;
        raw.push_back(m_mpi.GetDac()->rawFromValue(current));
    }
    return raw;
}

void Program::receivedPoints_cyclicTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_cyclicTest(points, Charts::Cyclic);
}

void Program::results_cyclicRegulatoryTests(const CyclicTestsRegulatory::TestResults& results)
{
    // auto &dst = m_telemetryStore.cyclicTestRecord;

    // // dst.sequence = results.strSequence;
    // // dst.ranges.resize(results.ranges.size());
    // // for (int i = 0; i < results.ranges.size(); ++i) {
    // //     const auto &src = results.ranges[i];
    // //     auto &d = dst.ranges[i];
    // //     d.rangePercent = src.rangePercent;
    // //     d.maxForwardValue = src.maxForwardValue;
    // //     d.maxForwardCycle = src.maxForwardCycle;
    // //     d.maxReverseValue = src.maxReverseValue;
    // //     d.maxReverseCycle = src.maxReverseCycle;
    // // }

    // emit telemetryUpdated(m_telemetryStore);
}

void Program::setMultipleDO(const QVector<bool>& states)
{
    quint8 mask = 0;
    for (int d = 0; d < states.size(); ++d) {
        m_mpi.SetDiscreteOutput(d, states[d]);
        if (states[d]) mask |= (1 << d);
    }
    //emit SetButtonsDOChecked(mask);
}

void Program::results_cyclicShutoffTests(const CyclicTestsShutoff::TestResults& results)
{
    auto &dst = m_telemetryStore.cyclicTestRecord;

    dst.doOnCounts  = results.doOnCounts;
    dst.doOffCounts = results.doOffCounts;

    dst.switch3to0Count = results.switch3to0Count;
    dst.switch0to3Count = results.switch0to3Count;

    emit telemetryUpdated(m_telemetryStore);

    // auto &dst = m_telemetryStore.cyclicTestRecord;

    // dst.sequence = "0-100-0";
    // dst.doOnCounts = results.doOnCounts;
    // dst.doOffCounts = results.doOffCounts;
    // dst.switch3to0Count = results.switch3to0Count / 2;
    // dst.switch0to3Count = results.switch0to3Count / 2;

    // emit telemetryUpdated(m_telemetryStore);
}

void Program::results_cyclicCombinedTests(const CyclicTestsRegulatory::TestResults& regulatoryResults,
                                          const CyclicTestsShutoff::TestResults& shutoffResults)
{
    // auto &dst = m_telemetryStore.cyclicTestRecord;
    // dst.sequence = dst.sequence;

    // dst.ranges.resize(regulatoryResults.ranges.size());
    // for (int i = 0; i < regulatoryResults.ranges.size(); ++i) {
    //     const auto &src = regulatoryResults.ranges[i];
    //     auto &d = dst.ranges[i];
    //     d.rangePercent = src.rangePercent;
    //     d.maxForwardValue = src.maxForwardValue;
    //     d.maxForwardCycle = src.maxForwardCycle;
    //     d.maxReverseValue = src.maxReverseValue;
    //     d.maxReverseCycle = src.maxReverseCycle;
    // }

    // dst.doOnCounts = shutoffResults.doOnCounts;
    // dst.doOffCounts = shutoffResults.doOffCounts;
    // dst.switch3to0Count = shutoffResults.switch3to0Count / 2;
    // dst.switch0to3Count = shutoffResults.switch0to3Count / 2;

    // emit telemetryUpdated(m_telemetryStore);
}

void Program::startCyclicTest()
{
    CyclicTestSettings::TestParameters parameters;
    emit getParameters_cyclicTest(parameters);
    qDebug() << "Program::runningCyclicTest testType =" << int(parameters.testType);

    if (parameters.regSeqValues.isEmpty() && parameters.offSeqValues.isEmpty())
        return;

    if (parameters.testType == CyclicTestSettings::TestParameters::Regulatory ||
        parameters.testType == CyclicTestSettings::TestParameters::Combined) {

        auto& rec = m_telemetryStore.cyclicTestRecord;

        QStringList parts;
        parts.reserve(parameters.regSeqValues.size());
        for (quint16 v : parameters.regSeqValues)
            parts << QString::number(v);
        rec.sequence = parts.join('-');

        rec.cycles = parameters.regulatory_numCycles;

        const quint64 stepsPerCycle = static_cast<quint64>(parameters.regSeqValues.size());
        const quint64 totalSteps    = stepsPerCycle * parameters.regulatory_numCycles;
        const quint64 totalMs       = totalSteps *
                                (parameters.regulatory_delayMs + parameters.regulatory_holdMs);
        rec.totalTimeSec = totalMs / 1000.0; // перевели в секунды

        // 4) Диапазоны для onCyclicStepMeasured
        rec.ranges.clear();
        rec.ranges.resize(parameters.regSeqValues.size());

        for (int i = 0; i < parameters.regSeqValues.size(); ++i) {
            auto& r = rec.ranges[i];
            r.rangePercent     = parameters.regSeqValues[i];

            // Прямой ход — будем искать максимум
            r.maxForwardValue  = std::numeric_limits<qreal>::lowest();
            r.maxForwardCycle  = -1;

            // Обратный ход — будем искать минимум
            r.maxReverseValue  = std::numeric_limits<qreal>::max();
            r.maxReverseCycle  = -1;
        }

        rec.switch3to0Count = 0;
        rec.switch0to3Count = 0;
    }

    switch (parameters.testType) {
    case CyclicTestSettings::TestParameters::Regulatory: {
        auto r = std::make_unique<CyclicRegulatoryRunner>(m_mpi, *m_registry, this);
        r->setParameters(parameters);
        startRunner(std::move(r));
        break;
    }
    case CyclicTestSettings::TestParameters::Shutoff: {
        auto r = std::make_unique<CyclicShutoffRunner>(m_mpi, *m_registry, this);
        r->setParameters(parameters);
        startRunner(std::move(r));
        break;
    }
    case CyclicTestSettings::TestParameters::Combined: {
        const quint64 regSteps = quint64(parameters.regSeqValues.size()) * parameters.regulatory_numCycles;
        const quint64 regMs = regSteps * (parameters.regulatory_delayMs + parameters.regulatory_holdMs)
                              + parameters.regulatory_delayMs;

        const quint64 offSteps = quint64(parameters.offSeqValues.size()) * parameters.shutoff_numCycles;
        const quint64 offMs = offSteps * (parameters.shutoff_delayMs + parameters.shutoff_holdMs)
                              + parameters.shutoff_delayMs;

        emit totalTestTimeMs(regMs + offMs);

        auto reg = std::make_unique<CyclicRegulatoryRunner>(m_mpi, *m_registry, this);
        reg->setParameters(parameters);

        connect(this, &Program::testFinished, this, [this, parameters]() {
            auto shut = std::make_unique<CyclicShutoffRunner>(m_mpi, *m_registry, this);
            shut->setParameters(parameters);
            startRunner(std::move(shut));
        }, Qt::SingleShotConnection);

        startRunner(std::move(reg));
        break;
    }
    default:
        break;
    }

    emit telemetryUpdated(m_telemetryStore);
}

void Program::onCyclicStepMeasured(int cycle, int step, bool forward)
{
    if (step < 0 || step >= m_telemetryStore.cyclicTestRecord.ranges.size())
        return;

    auto& rec = m_telemetryStore.cyclicTestRecord.ranges[step];

    const qreal measured = m_mpi[0]->percent();

    if (forward) {
        if (rec.maxForwardCycle < 0 || measured > rec.maxForwardValue) {
            rec.maxForwardValue = measured;
            rec.maxForwardCycle = cycle;
        }
    } else {
        if (rec.maxReverseCycle < 0 || measured < rec.maxReverseValue) {
            rec.maxReverseValue = measured;
            rec.maxReverseCycle = cycle;
        }
    }

    emit telemetryUpdated(m_telemetryStore);
}

void Program::results_cyclicTests(const CyclicTests::TestResults& r)
{
    auto& dst = m_telemetryStore.cyclicTestRecord;
    dst.sequence     = r.sequence;
    dst.cycles       = r.cycles;
    dst.totalTimeSec = r.totalTimeSec;

    dst.doOnCounts = r.doOnCounts;
    dst.doOffCounts = r.doOffCounts;
    dst.switch3to0Count = r.switch3to0Count / 2;
    dst.switch0to3Count = r.switch0to3Count / 2;

    emit telemetryUpdated(m_telemetryStore);
}

void Program::startOptionalTest(quint8 testNum)
{
    switch (testNum) {
    case 0: {
        auto r = std::make_unique<OptionResponseRunner>(m_mpi, *m_registry, this);
        connect(r.get(), &OptionResponseRunner::getParameters_responseTest,
                this, [&](OtherTestSettings::TestParameters& p){
                    emit getParameters_responseTest(p);
                });
        startRunner(std::move(r));
        break;
    }
    case 1: {
        auto r = std::make_unique<OptionResolutionRunner>(m_mpi, *m_registry, this);
        connect(r.get(), &OptionResolutionRunner::getParameters_resolutionTest,
                this, [&](OtherTestSettings::TestParameters& p){
                    emit getParameters_resolutionTest(p);
                });
        startRunner(std::move(r));
        break;
    }
    case 2: {
        auto r = std::make_unique<StepTestRunner>(m_mpi, *m_registry, this);
        startRunner(std::move(r));
        break;
    }
    default:
        emit stopTheTest();
        break;
    }
}

void Program::receivedPoints_stepTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_stepTest(points, Charts::Step);
}

void Program::results_stepTest(const QVector<StepTest::TestResult> &results, quint32 T_value)
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

    emit telemetryUpdated(m_telemetryStore);

    emit setStepResults(results, T_value);
}

void Program::updateCharts_optionTest(Charts chart)
{
    QVector<Point> points;

    qreal percent = calcPercent(m_mpi.GetDac()->value(),
                                m_registry->getValveInfo()->safePosition != 0);

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(chart, points);
}

void Program::button_set_position()
{
    m_isDacStopRequested = true;
    m_dacEventLoop->quit();
}

void Program::button_DO(quint8 DO_num, bool state)
{
    if (!m_isInitialized) {
        if ((int)m_initialDoStates.size() < 4)
            m_initialDoStates.resize(4);

        m_initialDoStates[DO_num] = state;

        quint8 mask = 0;
        for (int i = 0; i < m_initialDoStates.size(); ++i)
            if (m_initialDoStates[i]) mask |= (1 << i);

        emit setDoButtonsChecked(mask);
        return;
    }

    m_mpi.SetDiscreteOutput(DO_num, state);
    emit setDoButtonsChecked(m_mpi.GetDOStatus());
}

void Program::checkbox_autoInit(int state)
{
    m_shouldWaitForButton = (state == 0);
}

void Program::terminateTest()
{
    m_isDacStopRequested = true;
    m_dacEventLoop->quit();
    emit stopTheTest();
}
