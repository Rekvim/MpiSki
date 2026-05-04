#include "Program.h"

#include "Domain/Tests/Context.h"

#include "Domain/Tests/AbstractScenario.h"
#include "Domain/Tests/ScenarioFactory.h" // !

#include "Domain/Tests/Main/Params.h"
#include "Domain/Tests/Cyclic/Params.h"
#include "Domain/Tests/Option/Step/Params.h"
#include "Domain/Tests/Option/Params.h"

#include "Domain/Measurement/Sample.h"

#include "Domain/DeviceInitializer.h"

#include "Utils/SignalUtils.h"

#include <QDateTime>
#include <QLocale>
#include <utility>

namespace Domain {
constexpr quint8 VersionFlag = 0x40;


using ChartType = Widgets::Chart::ChartType;

Program::Program(QObject *parent)
    : QObject{parent}
{
    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);

    m_dacEventLoop = new QEventLoop(this);

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::updateSensors);

    m_timerDI = new QTimer(this);
    m_timerDI->setInterval(1000);
    connect(m_timerDI, &QTimer::timeout, this, [&]() {
        quint8 DI = m_device.digitalInputs();
        emit setDiCheckboxesChecked(DI);
    });
}

void Program::onRunnerActuallyStarted()
{
    m_isTestRunning = true;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_testDataBuffer.clear();

    emit testActuallyStarted();
}

void Program::setDacRaw(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart)
{
    m_isDacStopRequested = false;

    if (m_device.sensorCount() == 0) {
        emit releaseBlock();
        return;
    }

    m_device.setDacRaw(dac);

    if (waitForStart) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_device[0]->rawValue());
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
            lineSensor.push_back(m_device[0]->rawValue());
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

Measurement::Sample
Program::makeSample() const
{
    Measurement::Sample s;

    const auto& v = m_registry->valveInfo();
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    s.systemTime = now - m_initTime;
    s.testTime = m_isTestRunning ? now - m_startTime : 0;

    s.dac = m_device.dac()->value();

    const bool normalOpen = m_deviceConfig.safePosition == SafePosition::NormallyOpen;

    s.taskPercent = SignalUtils::calcPercent(s.dac, normalOpen);

    s.diMask = m_device.digitalInputs();
    s.doMask = m_device.digitalOutputs();

    if (auto* linear = m_device.sensorByAdc(0)) {
        s.positionValue = linear->value();
        s.positionPercent = linear->percent();
        s.positionUnit = (m_deviceConfig.strokeMovement == StrokeMovement::Rotary) ? "°" : "мм";
    }

    if (auto* p1 = m_device.sensorByAdc(1))
        s.pressure1 = p1->value();

    if (auto* p2 = m_device.sensorByAdc(2))
        s.pressure2 = p2->value();

    if (auto* p3 = m_device.sensorByAdc(3))
        s.pressure3 = p3->value();

    if (auto* fb = m_device.sensorByAdc(4))
        s.feedbackCurrent = fb->value();

    return s;
}

void Program::startScenario(std::unique_ptr<Domain::Tests::AbstractScenario> scenario)
{
    if (!isDeviceReadyForTest()) {
        failToStartTest("Нельзя запустить тест: устройство не инициализировано или не найдены датчики.");
        return;
    }

    if (!scenario) {
        failToStartTest("Нельзя запустить тест: сценарий не создан.");
        return;
    }

    m_currentScenario = std::move(scenario);

    connectScenarioRuntime(m_currentScenario.get());

    setDacRaw(0, 5000, true);

    m_currentScenario->start();
}

void Program::updateRealtimeTexts(const Domain::Measurement::Sample& s)
{
    if (m_isTestRunning && !qIsNaN(s.dac))
        emit setTask(s.dac);

    if (!qIsNaN(s.positionValue))
    {
        emit setText(
            TextObjects::LineEdit_linearSensor,
            QString("%1 %2").arg(s.positionValue, 0, 'f', 2).arg(s.positionUnit)
        );
    }

    if (!qIsNaN(s.positionPercent))
    {
        emit setText(
            TextObjects::LineEdit_linearSensorPercent,
            QString("%1 %").arg(s.positionPercent, 0, 'f', 2)
            );
    }

    if (!qIsNaN(s.pressure1))
    {
        emit setText(
            TextObjects::LineEdit_pressureSensor_1,
            QString("%1 bar").arg(s.pressure1, 0, 'f', 2)
            );
    }

    if (!qIsNaN(s.pressure2))
    {
        emit setText(
            TextObjects::LineEdit_pressureSensor_2,
            QString("%1 bar").arg(s.pressure2, 0, 'f', 2)
            );
    }

    if (!qIsNaN(s.pressure3))
    {
        emit setText(
            TextObjects::LineEdit_pressureSensor_3,
            QString("%1 bar").arg(s.pressure3, 0, 'f', 2)
            );
    }

    if (!qIsNaN(s.feedbackCurrent))
    {
        emit setText(
            TextObjects::LineEdit_feedback_4_20mA,
            QString("%1 mA").arg(s.feedbackCurrent, 0, 'f', 2)
            );
    }
}

void Program::updateMainCharts(const Measurement::Sample& s)
{
    QVector<Widgets::Chart::Point> points;

    if (auto* linear = m_device.sensorByAdc(0)) {
        const qreal x = s.dac;
        const qreal taskValue = linear->valueFromPercent(s.taskPercent);

        points.push_back({0, x, taskValue});
        points.push_back({1, x, linear->value()});
    }

    if (!qIsNaN(s.pressure1))
        points.push_back({2, s.dac, s.pressure1});

    if (!qIsNaN(s.pressure2))
        points.push_back({3, s.dac, s.pressure2});

    if (!qIsNaN(s.pressure3))
        points.push_back({4, s.dac, s.pressure3});

    emit addPoints(ChartType::Task, points);

    if (auto* linear = m_device.sensorByAdc(0)) {
        if (!qIsNaN(s.pressure1)) {
            QVector<Widgets::Chart::Point> pressurePoints;
            pressurePoints.push_back({0, s.pressure1, linear->value()});
            emit addPoints(ChartType::Pressure, pressurePoints);
        }
    }
}

void Program::updateCyclicChart(const Measurement::Sample& s, ChartType chart)
{
    QVector<Widgets::Chart::Point> points;
    points.push_back({0, qreal(s.testTime), s.taskPercent});
    points.push_back({1, qreal(s.testTime), s.positionPercent});

    emit addPoints(chart, points);

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        if (!m_telemetry.testСyclicShutoff)
            return;

        quint8 di = m_device.digitalInputs();

        if (di != m_lastDiStatus) {
            QVector<Widgets::Chart::Point> diPts;

            const bool lastClosed = (m_lastDiStatus & 0x01);
            const bool lastOpen = (m_lastDiStatus & 0x02);
            const bool nowClosed  = (di & 0x01);
            const bool nowOpen = (di & 0x02);

            if (nowClosed && !lastClosed) {
                ++m_telemetry.testСyclicShutoff->switch3to0Count;
                diPts.push_back({2, qreal(s.testTime), 0.0});
            } else if (!nowClosed && lastClosed) {
                diPts.push_back({2, qreal(s.testTime), 0.0});
            }

            if (nowOpen && !lastOpen) {
                ++m_telemetry.testСyclicShutoff->switch0to3Count;
                diPts.push_back({3, qreal(s.testTime), 100.0});
            } else if (!nowOpen && lastOpen) {
                diPts.push_back({3, qreal(s.testTime), 100.0});
            }

            if (!diPts.isEmpty())
                emit addPoints(chart, diPts);

            m_lastDiStatus = di;
        }
    }
}

void Program::updateSensors()
{
    const Measurement::Sample s = makeSample();

    emit sampleReady(s);

    if (m_isTestRunning && m_currentScenario)
        m_currentScenario->onSample(s);

    updateRealtimeTexts(s);
    updateChartsFromSample(s);
}

void Program::updateTimeChart(const Measurement::Sample& s, ChartType chart, qint64 time)
{
    QVector<Widgets::Chart::Point> points;

    points.push_back({0, qreal(time), s.taskPercent});
    points.push_back({1, qreal(time), s.positionPercent});

    emit addPoints(chart, points);
}

void Program::updateChartsFromSample(const Measurement::Sample& s)
{
    updateTimeChart(s, ChartType::Trend, s.systemTime);

    switch (m_testWorker)
    {
    case TestWorker::Stroke:
        updateTimeChart(s, ChartType::Stroke, s.testTime);
        break;

    case TestWorker::Main:
        updateMainCharts(s);
        break;

    case TestWorker::CyclicRegulatory:
        updateCyclicChart(s, ChartType::Cyclic);
        break;

    case TestWorker::CyclicShutOff:
        updateCyclicChart(s, ChartType::Cyclic);
        break;

    case TestWorker::Step:
        updateTimeChart(s, ChartType::Step, s.testTime);
        break;

    case TestWorker::Response:
        updateTimeChart(s, ChartType::Response, s.testTime);
        break;

    case TestWorker::Resolution:
        updateTimeChart(s, ChartType::Resolution, s.testTime);
        break;

    default:
        break;
    }
}

void Program::endTest()
{
    m_isTestRunning = false;
    m_testWorker = TestWorker::None;

    emit setTask(m_device.dac()->value());

    m_currentScenario.reset();

    m_isCyclicTestRunning = false;

    emit runnerFinished();

    if (!m_suppressPublicTestFinished) {
        emit testFinished();
    } else {
        m_suppressPublicTestFinished = false;
    }
}

void Program::setDacReal(qreal value)
{
    m_device.setDacValue(value);
}

void Program::setInitDoStates(const QVector<bool> &states)
{
    m_initialDoStates = states;
    m_savedInitialDoStates = states;
}

void Program::initialization()
{
    auto &ts = m_telemetry;

    m_isInitialized = false;

    ts.init.initStatusText = "";
    ts.init.connectedSensorsText = "";
    ts.init.deviceStatusText = "";
    ts.init.startingPositionText = "";
    ts.init.finalPositionText = "";

    m_timerSensors->stop();
    m_timerDI->stop();

    QString positionUnit = (m_deviceConfig.strokeMovement == StrokeMovement::Rotary) ? "°" : "мм";

    DeviceInitializer initializer(
        m_device,
        m_telemetry,
        {
            .normalClosed = m_deviceConfig.safePosition == SafePosition::NormallyClosed,
            .strokeMovement = m_deviceConfig.strokeMovement,
            .diameterPulley = m_deviceConfig.diameterPulley
        }
    );

    if (!initializer.connectAndInitDevice()) {
        emit telemetryUpdated(m_telemetry);
        return;
    } emit telemetryUpdated(m_telemetry);

    if (!initializer.detectSensors()) {
        emit telemetryUpdated(m_telemetry);
        return;
    } emit telemetryUpdated(m_telemetry);

    if (m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT ||
        m_patternType == SelectTests::Pattern_C_SOVT) {

        if ((m_device.version() & VersionFlag) != 0) {
            emit setDoButtonsChecked(m_device.digitalOutputs());
            m_timerDI->start();
        } else {
            return;
        }

        setDacRaw(65535, 10000, true);
        waitForDacCycle();
        initializer.measureEndPositionShutoff(
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetry);

        setDacRaw(0, 10000, true);
        waitForDacCycle();
        initializer.measureStartPositionShutoff(
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetry);
    }

    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT) {

        setDacRaw(0, 10000, true);
        waitForDacCycle();
        initializer.measureStartPosition();
        emit telemetryUpdated(m_telemetry);

        setDacRaw(65535, 10000, true);
        waitForDacCycle();
        initializer.measureEndPosition();
        emit telemetryUpdated(m_telemetry);
    }

    initializer.calculateCoefficients();

    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {
        initializer.recordStrokeRange();

        setDacRaw(0, 10000, true);

        emit telemetryUpdated(m_telemetry);
    }

    finalizeInitialization();
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

void Program::finalizeInitialization()
{
    emit clearPoints(ChartType::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    quint8 mask = 0;
    for (quint8 adc = 0; adc < 6; ++adc) {
        if (m_device.sensorByAdc(adc))
            mask |= (1 << adc);
    }

    emit setSensorsMask(mask);
    emit setSensorNumber(m_device.sensorCount());
    m_isInitialized = true;
    m_timerSensors->start();
}

void Program::startMainTest(const Tests::Main::Params& params)
{
    m_testWorker = TestWorker::Main;

    emit clearPoints(ChartType::Trend);
    emit clearPoints(ChartType::Pressure);
    emit clearPoints(ChartType::Friction);
    emit setRegressionEnable(false);

    auto scenario = Tests::ScenarioFactory::createMain(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::addFriction(const QVector<QPointF> &points)
{
    QVector<Widgets::Chart::Point> chartPoints;

    qreal k = 5 * M_PI * m_deviceConfig.driveDiameter * m_deviceConfig.driveDiameter / 4;

    for (QPointF point : points) {
        chartPoints.push_back({0, point.x(), point.y() * k});
    }
    emit addPoints(ChartType::Friction, chartPoints);
}

void Program::addRegression(const QVector<QPointF> &points)
{
    QVector<Widgets::Chart::Point> chartPoints;
    for (QPointF point : points) {
        chartPoints.push_back({1, point.x(), point.y()});
    }
    emit addPoints(ChartType::Pressure, chartPoints);

    emit setRegressionEnable(true);
}

bool Program::isDeviceReadyForTest() const
{
    if (!m_isInitialized) {
        qWarning() << "[Program] Cannot start test: device is not initialized";
        return false;
    }

    if (m_device.sensorCount() == 0) {
        qWarning() << "[Program] Cannot start test: no sensors detected";
        return false;
    }

    return true;
}

void Program::startStrokeTest()
{
    m_testWorker = TestWorker::Stroke;

    auto scenario = Tests::ScenarioFactory::createStroke(
        makeContext(),
        this
    );

    startScenario(std::move(scenario));
}

void Program::failToStartTest(const QString& reason)
{
    qWarning().noquote() << "[Program] Test start rejected:" << reason;

    // emit errorOccured(reason);
    emit testStartRejected(reason);

    m_isTestRunning = false;
    m_testWorker = TestWorker::None;

    m_currentScenario.reset();
}

QVector<quint16> Program::makeRawValues(const QVector<quint16> &seq, bool normalOpen)
{
    QVector<quint16> raw;
    raw.reserve(seq.size());

    for (quint16 pct : seq) {
        qreal current = 16.0 * (normalOpen ? 100 - pct : pct) / 100.0 + 4.0;
        raw.push_back(m_device.dac()->rawFromValue(current));
    }
    return raw;
}

void Program::setMultipleDO(const QVector<bool>& states)
{
    quint8 mask = 0;
    for (int d = 0; d < states.size(); ++d) {
        m_device.setDiscreteOutput(d, states[d]);
        if (states[d]) mask |= (1 << d);
    }
    //emit SetButtonsDOChecked(mask);
}

Tests::Context Program::makeContext()
{
    return Tests::Context {
        .device = m_device,
        .telemetry = m_telemetry,
        .config = m_deviceConfig
    };
}

void Program::prepareRegulatoryTelemetry(const Tests::Cyclic::Params& params)
{
    auto& result = m_telemetry.testСyclicRegulatory.emplace();
    const auto& paramsRegulatory = params.regulatory;
    QStringList parts;
    parts.reserve(paramsRegulatory.sequence.size());

    for (const quint16 v : std::as_const(paramsRegulatory.sequence))
        parts << QString::number(v);

    result.sequence = parts.join('-');
    result.numCycles = paramsRegulatory.numCycles;

    const quint64 stepsPerCycle = paramsRegulatory.sequence.size();
    const quint64 totalSteps = stepsPerCycle * paramsRegulatory.numCycles;
    const quint64 totalMs = totalSteps * (paramsRegulatory.delayMs + paramsRegulatory.holdMs);
    result.totalTimeSec = totalMs / 1000.0;
}

void Program::prepareShutoffTelemetry(const Tests::Cyclic::Params& params)
{
    auto& result = m_telemetry.testСyclicShutoff.emplace();
    const auto& paramsShutoff = params.shutoff;
    QStringList parts;
    parts.reserve(paramsShutoff.sequence.size());

    for (const quint16 v : std::as_const(paramsShutoff.sequence))
        parts << QString::number(v);

    result.sequence = parts.join('-');

    result.numCycles = paramsShutoff.numCycles;

    const quint64 stepsPerCycle = paramsShutoff.sequence.size();
    const quint64 totalSteps = stepsPerCycle * paramsShutoff.numCycles;

    const quint64 totalMs =
        totalSteps * (paramsShutoff.delayMs + paramsShutoff.holdMs);

    result.totalTimeSec = totalMs / 1000.0;
}

void Program::startCyclicRegulatoryScenario(const Tests::Cyclic::Regulatory::Params& params)
{
    m_testWorker = TestWorker::CyclicRegulatory;

    auto scenario = Tests::ScenarioFactory::createCyclicRegulatory(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::startCyclicShutoffScenario(const Tests::Cyclic::Shutoff::Params& params)
{
    m_testWorker = TestWorker::CyclicShutOff;

    auto scenario = Tests::ScenarioFactory::createCyclicShutoff(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::runCombinedCyclicTest(const Tests::Cyclic::Params& params)
{
    const auto& regP = params.regulatory;
    const auto& shutP = params.shutoff;

    const quint64 regSteps =
        static_cast<quint64>(regP.sequence.size()) *
        static_cast<quint64>(regP.numCycles);

    const quint64 regMs =
        regSteps *
        static_cast<quint64>(regP.delayMs + regP.holdMs);

    const quint64 shutSteps =
        static_cast<quint64>(shutP.sequence.size()) *
        static_cast<quint64>(shutP.numCycles);

    const quint64 shutMs =
        shutSteps *
        static_cast<quint64>(shutP.delayMs + shutP.holdMs);

    emit totalTestTimeMs(regMs + shutMs);

    connect(this, &Program::runnerFinished,
            this, [this, params]() {
                startCyclicShutoffScenario(params.shutoff);
            },
            Qt::SingleShotConnection);

    m_suppressPublicTestFinished = true;

    startCyclicRegulatoryScenario(params.regulatory);
}

void Program::startCyclicTest(const Tests::Cyclic::Params& params)
{
    if (params.regulatory.sequence.isEmpty() &&
        params.shutoff.sequence.isEmpty()) {
        failToStartTest("Cyclic test: обе последовательности пустые.");
        return;
    }

    if (params.type == Tests::Cyclic::Params::Regulatory &&
        params.regulatory.sequence.isEmpty()) {
        failToStartTest("Cyclic regulatory test: последовательность пуста.");
        return;
    }

    if (params.type == Tests::Cyclic::Params::Shutoff &&
        params.shutoff.sequence.isEmpty()) {
        failToStartTest("Cyclic shutoff test: последовательность пуста.");
        return;
    }

    if (params.type == Tests::Cyclic::Params::Combined) {
        if (params.regulatory.sequence.isEmpty()) {
            failToStartTest("Combined cyclic test: regulatory-последовательность пуста.");
            return;
        }

        if (params.shutoff.sequence.isEmpty()) {
            failToStartTest("Combined cyclic test: shutoff-последовательность пуста.");
            return;
        }
    }

    if (params.type == Tests::Cyclic::Params::Regulatory ||
        params.type == Tests::Cyclic::Params::Combined) {
        prepareRegulatoryTelemetry(params);
    }

    if (params.type == Tests::Cyclic::Params::Shutoff ||
        params.type == Tests::Cyclic::Params::Combined) {
        prepareShutoffTelemetry(params);
    }

    switch (params.type)
    {
    case Tests::Cyclic::Params::Regulatory:
        startCyclicRegulatoryScenario(params.regulatory);
        break;

    case Tests::Cyclic::Params::Shutoff:
        startCyclicShutoffScenario(params.shutoff);
        break;

    case Tests::Cyclic::Params::Combined:
        runCombinedCyclicTest(params);
        break;

    default:
        break;
    }

    emit telemetryUpdated(m_telemetry);
}

void Program::connectScenarioRuntime(Domain::Tests::AbstractScenario* scenario)
{
    Q_ASSERT(scenario);

    connect(scenario, &Domain::Tests::AbstractScenario::requestClearChart,
            this, [this](Widgets::Chart::ChartType chartType) {
                emit clearPoints(chartType);
            });

    connect(scenario, &Domain::Tests::AbstractScenario::started,
            this, &Program::onRunnerActuallyStarted);

    connect(scenario, &Domain::Tests::AbstractScenario::requestSetDAC,
            this, &Program::setDacRaw);

    connect(this, &Program::releaseBlock,
            scenario, &Domain::Tests::AbstractScenario::releaseBlock);

    connect(scenario, &Domain::Tests::AbstractScenario::totalTestTimeMs,
            this, &Program::totalTestTimeMs);

    connect(scenario, &Domain::Tests::AbstractScenario::finished,
            this, &Program::endTest);

    connect(scenario, &Domain::Tests::AbstractScenario::failedToStart,
            this, &Program::failToStartTest);

    connect(this, &Program::stopTheTest,
            scenario, &Domain::Tests::AbstractScenario::stop);

    connect(scenario, &Domain::Tests::AbstractScenario::telemetryUpdated,
            this, &Program::telemetryUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::mainResultUpdated,
            this, &Program::mainResultUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::strokeResultUpdated,
            this, &Program::strokeResultUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::stepResultUpdated,
            this, &Program::stepResultUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::cyclicRegulatoryResultUpdated,
            this, &Program::cyclicRegulatoryResultUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::cyclicShutoffResultUpdated,
            this, &Program::cyclicShutoffResultUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::crossingStatusUpdated,
            this, &Program::crossingStatusUpdated,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::pointsRequested,
            this, [this](QVector<QVector<QPointF>>& pointsF,
                   Widgets::Chart::ChartType chartType) {
                points(pointsF, chartType);
            }, Qt::DirectConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::addRegressionRequested,
            this, &Program::addRegression,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::addFrictionRequested,
            this, &Program::addFriction,
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::duplicateMainChartsSeriesRequested,
            this, [this] {
                emit duplicateMainChartsSeries();
            },
            Qt::QueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::cyclicCycleCompleted,
            this, &Program::cyclicCycleCompleted,
            Qt::BlockingQueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::setMultipleDORequested,
            this, &Program::setMultipleDO,
            Qt::BlockingQueuedConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::diRequested,
            this, [this](quint8& status) {
                status = getDIStatus();
            },
            Qt::DirectConnection);

    connect(scenario, &Domain::Tests::AbstractScenario::doRequested,
            this, [this](quint8& status) {
                status = getDOStatus();
            },
            Qt::DirectConnection);
}

void Program::startResponseTest(const Tests::Option::Params& params)
{
    m_testWorker = TestWorker::Response;

    if (params.points.isEmpty()) {
        failToStartTest("Response test: список точек пуст.");
        return;
    }

    if (params.steps.isEmpty()) {
        failToStartTest("Response test: список шагов пуст.");
        return;
    }

    auto scenario = Tests::ScenarioFactory::createResponse(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::startResolutionTest(const Tests::Option::Params& params)
{
    if (params.points.isEmpty()) {
        failToStartTest("Resolution test: список точек пуст.");
        return;
    }

    if (params.steps.isEmpty()) {
        failToStartTest("Resolution test: список шагов пуст.");
        return;
    }

    m_testWorker = TestWorker::Resolution;

    auto scenario = Tests::ScenarioFactory::createResolution(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::startStepTest(const Tests::Option::Step::Params& params)
{
    m_testWorker = TestWorker::Step;

    if (params.points.isEmpty()) {
        failToStartTest("Step test: список точек пуст.");
        return;
    }

    auto scenario = Tests::ScenarioFactory::createStep(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
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

    m_device.setDiscreteOutput(DO_num, state);
    emit setDoButtonsChecked(m_device.digitalOutputs());
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
}