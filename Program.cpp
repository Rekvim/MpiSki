#include "Program.h"

#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Src/Tests/MainTest.h"
#include "Src/Utils/Number.h"

#include "./Src/Runners/MainTestRunner.h"
#include "./Src/Runners/StepTestRunner.h"
#include "./Src/Runners/StrokeTestRunner.h"
#include "./Src/Runners/OptionResponseRunner.h"
#include "./Src/Runners/OptionResolutionRunner.h"
#include "./Src/Runners/CyclicRegulatoryRunner.h"
#include "./Src/Runners/CyclicShutoffRunner.h"

#include <QRegularExpression>
#include <QLocale>
#include <utility>

constexpr quint8 VersionFlag = 0x40;

Program::Program(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<RealtimeState>("RealtimeState");

    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);

    m_dacEventLoop = new QEventLoop(this);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::updateSensors);

    m_timerDI = new QTimer(this);
    m_timerDI->setInterval(1000);
    connect(m_timerDI, &QTimer::timeout, this, [&]() {
        quint8 DI = m_mpi.digitalInputs();
        emit setDiCheckboxesChecked(DI);
    });
}

void Program::setRegistry(Registry *registry)
{
    m_registry = registry;
}

void Program::setDacRaw(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    m_isDacStopRequested = false;

    if (m_mpi.sensorCount() == 0) {
        emit releaseBlock();
        return;
    }

    m_mpi.setDacRaw(dac);

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
    for (quint8 adc = 0; adc < 6; ++adc) {
        Sensor* s = m_mpi.sensorByAdc(adc);
        if (!s) continue;
    }
    if (m_isTestRunning)
        emit setTask(m_mpi.dac()->value());

    if (auto s = m_mpi.sensorByAdc(0)) {
        emit setText(TextObjects::LineEdit_linearSensor, s->formattedValue());
        emit setText(TextObjects::LineEdit_linearSensorPercent, s->percentFormatted());
    }

    if (auto s = m_mpi.sensorByAdc(1))
        emit setText(TextObjects::LineEdit_pressureSensor_1, s->formattedValue());

    if (auto s = m_mpi.sensorByAdc(2))
        emit setText(TextObjects::LineEdit_pressureSensor_2, s->formattedValue());

    if (auto s = m_mpi.sensorByAdc(3))
        emit setText(TextObjects::LineEdit_pressureSensor_3, s->formattedValue());

    if (auto s = m_mpi.sensorByAdc(4))
        emit setText(TextObjects::LineEdit_feedback_4_20mA, s->formattedValue());

    QVector<Point> points;

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition == SafePosition::NormallyOpen
        );

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

    emit setTask(m_mpi.dac()->value());

    m_activeRunner.reset();

    m_isCyclicTestRunning = false;
    emit testFinished();
}

void Program::disposeActiveRunnerAsync() {
    m_activeRunner.reset();
}

void Program::setDAC_real(qreal value)
{
    m_mpi.setDacValue(value);
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

    if (!connectAndInitDevice()) {
        emit setButtonInitEnabled(true);
        return;
    }

    if (!detectAndReportSensors()) {
        emit setButtonInitEnabled(true);
        return;
    }

    const auto& valveInfo = m_registry->valveInfo();
    bool normalClosed = (valveInfo.safePosition == SafePosition::NormallyClosed);

    // Измерение начального и конечного положения соленоида
    if (m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT ||
        m_patternType == SelectTests::Pattern_C_SOVT) {

        if ((m_mpi.version() & VersionFlag) != 0) {
            emit setDoButtonsChecked(m_mpi.digitalOutputs());
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
    bool ok = m_mpi.isConnect();
    ts.init.deviceStatusText  = ok
                                   ? QString("Успешное подключение к порту %1").arg(m_mpi.portName())
                                   : "Ошибка подключения";
    ts.init.deviceStatusColor = ok ? Qt::darkGreen : Qt::red;
    emit telemetryUpdated(ts);

    if (!ok) {
        return false;
    }

    ok = m_mpi.initialize();
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
    int cnt = m_mpi.sensorCount();

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
    connect(&timer, &QTimer::timeout, this, [this] {
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

    setDacRaw(65535, 10000, true);

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
            m_mpi.setDiscreteOutput(i, false);
        }
    }
    emit setDoButtonsChecked(m_mpi.digitalOutputs());

    setDacRaw(0, 1000, true, true);

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

    s.init.finalPositionText = "Измерение";
    s.init.finalPositionColor = Qt::darkYellow;
    emit telemetryUpdated(s);

    for (int i = 0; i < m_savedInitialDoStates.size(); ++i) {
        if (m_savedInitialDoStates[i]) {
            m_initialDoStates[i] = true;
            m_mpi.setDiscreteOutput(i, true);
        }
    }
    emit setDoButtonsChecked(m_mpi.digitalOutputs());

    setDacRaw(65535, 1000, true);

    waitForDacCycle();

    if (normalClosed) m_mpi[0]->captureMax();
    else m_mpi[0]->captureMin();

    s.init.finalPositionText = m_mpi[0]->formattedValue();
    s.init.finalPositionColor = Qt::darkGreen;
    emit telemetryUpdated(s);
}

void Program::calculateAndApplyCoefficients()
{
    const auto& valveInfo = m_registry->valveInfo();

    qreal coeff = 1.0;

    if (valveInfo.strokeMovement == StrokeMovement::Rotary) {
        coeff = qRadiansToDegrees(2.0 / valveInfo.diameterPulley);
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
    emit setTask(m_mpi.dac()->value());
}

void Program::finalizeInitialization()
{
    emit clearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    quint8 mask = 0;
    for (quint8 adc = 0; adc < 6; ++adc) {
        if (m_mpi.sensorByAdc(adc))
            mask |= (1 << adc);
    }

    emit setSensorsMask(mask);
    emit setSensorNumber(m_mpi.sensorCount());
    emit setButtonInitEnabled(true);

    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}

void Program::startMainTest(const MainTestSettings::TestParameters& params)
{
    auto r = std::make_unique<MainTestRunner>(
        m_mpi,
        *m_registry,
        params,
        this
    );

    startRunner(std::move(r));
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

void Program::updateCrossingStatus()
{
    auto &ts = m_telemetryStore;
    const auto& valveInfo = m_registry->valveInfo();
    const auto& limits = valveInfo.crossingLimits;
    using State = CrossingStatus::State;

    if (limits.frictionEnabled) {
        ts.crossingStatus.frictionPercent =
            inRange(ts.mainTestRecord.frictionPercent,
                    limits.frictionCoefLower,
                    limits.frictionCoefUpper)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.frictionPercent = State::Unknown;
    }

    if (limits.valveStrokeEnabled) {
        bool ok = false;
        const double recStroke = NumberUtils::toDouble(valveInfo.valveStroke, &ok);
        if (ok) {
            const double d = std::abs(recStroke) * (limits.valveStroke / 100.0); // rangeUpperLimit как %
            const double lo = recStroke - d;
            const double hi = recStroke + d;

            ts.crossingStatus.valveStroke =
                inRange(ts.valveStrokeRecord.real, lo, hi) ? State::Ok : State::Fail;
        } else {
            ts.crossingStatus.valveStroke = State::Unknown;
        }
    } else {
        ts.crossingStatus.valveStroke = State::Unknown;
    }

    if (limits.dynamicErrorEnabled) {
        ts.crossingStatus.dynamicError =
            inRange(ts.mainTestRecord.dynamicErrorReal,
                    0.0,
                                                 QString(valveInfo.dinamicErrorRecomend).toDouble())
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.dynamicError = State::Unknown;
    }

    if (limits.springEnabled) {
        double recLow  = valveInfo.driveRangeLow;
        double recHigh = valveInfo.driveRangeHigh;

        if (recLow > recHigh)
            std::swap(recLow, recHigh);

        const double lowD  = std::abs(recLow) * (limits.springLower / 100.0);
        const double highD = std::abs(recHigh) * (limits.springUpper / 100.0);

        const double lowLo = recLow - lowD;
        const double lowHi = recLow + lowD;

        const double highLo = recHigh - highD;
        const double highHi = recHigh + highD;

        const bool okLow  = inRange(ts.mainTestRecord.springLow,  lowLo,  lowHi);
        const bool okHigh = inRange(ts.mainTestRecord.springHigh, highLo, highHi);

        ts.crossingStatus.spring = (okLow && okHigh)
                                       ? State::Ok
                                       : State::Fail;
    } else {
        ts.crossingStatus.spring = State::Unknown;
    }

    // --- linearCharacteristic ---
    if (limits.linearCharacteristicEnabled) {
        ts.crossingStatus.linearCharacteristic =
            inRange(ts.mainTestRecord.linearityError, 0.0, limits.linearCharacteristic)
                ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.linearCharacteristic = State::Unknown;
    }
}

void Program::results_mainTest(const MainTest::TestResults &results)
{
    const auto& valveInfo = m_registry->valveInfo();
    qreal k = 5 * M_PI * valveInfo.driveDiameter * valveInfo.driveDiameter / 4;

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

    const auto& v = m_registry->valveInfo();
    const qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition == SafePosition::NormallyOpen
        );

    const qreal X = m_mpi.dac()->value();

    if (auto* linear = m_mpi.sensorByAdc(0)) {
        const qreal task = linear->valueFromPercent(percent);

        // series 0 = задание
        points.push_back({0, X, task});

        // series 1 = линейный датчик
        points.push_back({1, X, linear->value()});
    }

    // series 2 = давление 1 (adc1)
    if (auto* p1 = m_mpi.sensorByAdc(1))
        points.push_back({2, X, p1->value()});

    // series 3 = давление 2 (adc2)
    if (auto* p2 = m_mpi.sensorByAdc(2))
        points.push_back({3, X, p2->value()});

    // series 4 = давление 3 (adc3)
    if (auto* p3 = m_mpi.sensorByAdc(3))
        points.push_back({4, X, p3->value()});

    emit addPoints(Charts::Task, points);

    // График "Перемещение от давления" строим только если есть linear + pressure1
    if (auto* linear = m_mpi.sensorByAdc(0)) {
        if (auto* p1 = m_mpi.sensorByAdc(1)) {
            QVector<Point> pressurePoints;
            pressurePoints.push_back({0, p1->value(), linear->value()});
            emit addPoints(Charts::Pressure, pressurePoints);
        }
    }
}

void Program::addFriction(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;

    auto& valveInfo = m_registry->valveInfo();

    qreal k = 5 * M_PI * valveInfo.driveDiameter * valveInfo.driveDiameter / 4;

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

void Program::receivedPoints_strokeTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_strokeTest(points, Charts::Stroke);
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

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition == SafePosition::NormallyOpen
        );

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});

    emit addPoints(Charts::Stroke, points);
}

void Program::updateCharts_CyclicTest(Charts chart)
{
    QVector<Point> points;

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition == SafePosition::NormallyOpen
        );

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->percent()});
    emit addPoints(chart, points);

    if (m_patternType == SelectTests::Pattern_C_SOVT  ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {
        quint8 di = m_mpi.digitalInputs();
        if (di != m_lastDiStatus) {
            QVector<Point> diPts;
            bool lastClosed = (m_lastDiStatus & 0x01);
            bool lastOpen = (m_lastDiStatus & 0x02);
            bool nowClosed = (di & 0x01);
            bool nowOpen = (di & 0x02);
            if (nowClosed && !lastClosed) {
                ++m_telemetryStore.cyclicTestRecord.switch3to0Count;
                diPts.push_back({2, qreal(time), 0.0});
            } else if (!nowClosed && lastClosed) {
                diPts.push_back({2, qreal(time), 0.0});
            }

            if (nowOpen && !lastOpen) {
                ++m_telemetryStore.cyclicTestRecord.switch0to3Count;
                diPts.push_back({3, qreal(time), 100.0});
            } else if (!nowOpen && lastOpen) {
                diPts.push_back({3, qreal(time), 100.0});
            }

            if (!diPts.isEmpty()) emit addPoints(chart, diPts);
            m_lastDiStatus = di;
        }
    }
}

QVector<quint16> Program::makeRawValues(const QVector<quint16> &seq, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.size());

    for (quint16 pct : seq) {
        qreal current = 16.0 * (normalOpen ? 100 - pct : pct) / 100.0 + 4.0;
        raw.push_back(m_mpi.dac()->rawFromValue(current));
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
        m_mpi.setDiscreteOutput(d, states[d]);
        if (states[d]) mask |= (1 << d);
    }
    //emit SetButtonsDOChecked(mask);
}

void Program::results_cyclicShutoffTests(const CyclicTestsShutoff::TestResults& results)
{
    auto &dst = m_telemetryStore.cyclicTestRecord;

    dst.numCyclesShutoff = results.numCycles;
    dst.doOnCounts = results.doOnCounts;
    dst.doOffCounts = results.doOffCounts;

    dst.switch3to0Count = results.switch3to0Count;
    dst.switch0to3Count = results.switch0to3Count;

    emit telemetryUpdated(m_telemetryStore);
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

void Program::startCyclicTest(const CyclicTestSettings::TestParameters& params)
{
    if (params.regSeqValues.isEmpty() && params.offSeqValues.isEmpty()) {
        emit testFinished();
        return;
    }

    if (params.testType == CyclicTestSettings::TestParameters::Regulatory ||
        params.testType == CyclicTestSettings::TestParameters::Combined) {

        auto& rec = m_telemetryStore.cyclicTestRecord;

        QStringList parts;
        parts.reserve(params.regSeqValues.size());

        for (const quint16 v : std::as_const(params.regSeqValues))
            parts << QString::number(v);

        rec.sequenceRegulatory = parts.join('-');

        rec.numCyclesRegulatory = params.regulatory_numCycles;

        const quint64 stepsPerCycle = static_cast<quint64>(params.regSeqValues.size());
        const quint64 totalSteps = stepsPerCycle * params.regulatory_numCycles;
        const quint64 totalMs = totalSteps *
                                (params.regulatory_delayMs + params.regulatory_holdMs);
        rec.totalTimeSecRegulatory = totalMs / 1000.0;

        rec.ranges.clear();
        rec.ranges.resize(params.regSeqValues.size());

        for (int i = 0; i < params.regSeqValues.size(); ++i) {
            auto& r = rec.ranges[i];
            r.rangePercent     = params.regSeqValues[i];

            r.maxForwardValue  = std::numeric_limits<qreal>::lowest();
            r.maxForwardCycle  = -1;

            r.maxReverseValue  = std::numeric_limits<qreal>::max();
            r.maxReverseCycle  = -1;
        }
    }

    if (params.testType == CyclicTestSettings::TestParameters::Shutoff) {

        auto& rec = m_telemetryStore.cyclicTestRecord;

        QStringList parts;
        parts.reserve(params.offSeqValues.size());

        for (const quint16 v : std::as_const(params.offSeqValues))
            parts << QString::number(v);

        rec.sequenceShutoff = parts.join('-');

        rec.numCyclesShutoff = params.shutoff_numCycles;

        const quint64 stepsPerCycle = static_cast<quint64>(params.offSeqValues.size());
        const quint64 totalSteps = stepsPerCycle * params.shutoff_numCycles;
        const quint64 totalMs = totalSteps *
                                (params.shutoff_delayMs + params.shutoff_holdMs);
        rec.totalTimeSecShutoff = totalMs / 1000.0;
    }

    switch (params.testType) {
    case CyclicTestSettings::TestParameters::Regulatory: {
        auto r = std::make_unique<CyclicRegulatoryRunner>(m_mpi, *m_registry, params, this);
        // r->setParameters(params);
        startRunner(std::move(r));
        break;
    }
    case CyclicTestSettings::TestParameters::Shutoff: {
        auto r = std::make_unique<CyclicShutoffRunner>(m_mpi, *m_registry, params, this);
        // r->setParameters(params);
        startRunner(std::move(r));
        break;
    }
    case CyclicTestSettings::TestParameters::Combined: {
        const quint64 regSteps = quint64(params.regSeqValues.size()) * params.regulatory_numCycles;
        const quint64 regMs = regSteps * (params.regulatory_delayMs + params.regulatory_holdMs)
                              + params.regulatory_delayMs;

        const quint64 offSteps = quint64(params.offSeqValues.size()) * params.shutoff_numCycles;
        const quint64 offMs = offSteps * (params.shutoff_delayMs + params.shutoff_holdMs)
                              + params.shutoff_delayMs;

        emit totalTestTimeMs(regMs + offMs);

        auto reg = std::make_unique<CyclicRegulatoryRunner>(m_mpi, *m_registry,  params, this);
        // reg->setParameters(params);

        connect(this, &Program::testFinished, this, [this, params]() {
            auto shut = std::make_unique<CyclicShutoffRunner>(m_mpi, *m_registry, params, this);
            // shut->setParameters(parameters);
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

void Program::startOptionalTest(quint8 testNum)
{

}

void Program::startResponseTest(const OtherTestSettings::TestParameters& params) {
    auto r = std::make_unique<OptionResponseRunner>(m_mpi, *m_registry, params, this);

    startRunner(std::move(r));
}

void Program::startResolutionTest(const OtherTestSettings::TestParameters& params) {
    auto r = std::make_unique<OptionResolutionRunner>(m_mpi, *m_registry, params, this);
    startRunner(std::move(r));
}
void Program::startStepTest(const StepTestSettings::TestParameters& params) {
    auto r = std::make_unique<StepTestRunner>(m_mpi, *m_registry, params, this);
    startRunner(std::move(r));
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

    const auto& v = m_registry->valveInfo();
    qreal percent = calcPercent(
        m_mpi.dac()->value(),
        v.safePosition == SafePosition::NormallyOpen
    );

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

    m_mpi.setDiscreteOutput(DO_num, state);
    emit setDoButtonsChecked(m_mpi.digitalOutputs());
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
