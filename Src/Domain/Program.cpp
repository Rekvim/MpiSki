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
#include <QDebug>

#include <utility>

namespace Domain {

namespace {
constexpr quint16 kDacRawMin = 0;
constexpr quint16 kDacRawMax = 65535;
constexpr quint8 VersionFlag = 0x40;

}


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

    connect(&m_device, &Mpi::Device::errorOccured,
            this, [this](const QString& msg) {
                m_telemetry.init.deviceStatusText = msg;
                m_telemetry.init.deviceStatusColor = Qt::red;
                emit telemetryUpdated(m_telemetry);
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

    setDacRaw(kDacRawMin, 5000, true);

    m_currentScenario->start();
}

void Program::updateSensors()
{
    const Measurement::Sample s = makeSample();

    emit sampleReady(s);

    emit addPoints(ChartType::Trend, {
        {0, qreal(s.systemTime), s.taskPercent},
        {1, qreal(s.systemTime), s.positionPercent}
    });

    if (m_isTestRunning && m_currentScenario) {
        m_currentScenario->onSample(s);
        m_currentScenario->updateChart(s);
    }
}

void Program::endTest()
{
    m_isTestRunning = false;

    emit setTask(m_device.dac()->value());

    emit runnerFinished();

    m_currentScenario.reset();

    m_isCyclicTestRunning = false;

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

    QString positionUnit =
        (m_deviceConfig.strokeMovement == StrokeMovement::Rotary) ? "°" : "мм";

    DeviceInitializer initializer(
        m_device,
        m_telemetry,
        {
            m_deviceConfig.safePosition == SafePosition::NormallyClosed,
            m_deviceConfig.strokeMovement,
            m_deviceConfig.diameterPulley
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

    if (m_deviceProfile.hasShutoff()) {
        if ((m_device.version() & VersionFlag) != 0) {
            emit setDoButtonsChecked(m_device.digitalOutputs());
            m_timerDI->start();
        } else {
            return;
        }

        setDacRaw(kDacRawMax, 10000, true);
        waitForDacCycle();
        initializer.measureEndPositionShutoff(
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetry);

        setDacRaw(kDacRawMin, 10000, true);
        waitForDacCycle();
        initializer.measureStartPositionShutoff(
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetry);
    }

    if (m_deviceProfile.hasControl()) {
        setDacRaw(kDacRawMin, 10000, true);
        waitForDacCycle();
        initializer.measureStartPosition();
        emit telemetryUpdated(m_telemetry);

        setDacRaw(kDacRawMax, 10000, true);
        waitForDacCycle();
        initializer.measureEndPosition();
        emit telemetryUpdated(m_telemetry);
    }

    initializer.calculateCoefficients();

    if (m_deviceProfile.hasControl()) {
        initializer.recordStrokeRange();

        setDacRaw(kDacRawMin, 10000, true);

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

    qreal k = 5 * M_PI * m_deviceConfig.driveDiameter
              * m_deviceConfig.driveDiameter / 4;

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
    auto scenario = Tests::ScenarioFactory::createStroke(
        makeContext(),
        this
    );

    startScenario(std::move(scenario));
}

void Program::failToStartTest(const QString& reason)
{
    qWarning().noquote() << "[Program] Test start rejected:" << reason;

    emit testStartRejected(reason);

    m_isTestRunning = false;
    m_currentScenario.reset();
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
    return Tests::Context{
        m_device,
        m_telemetry,
        m_deviceConfig
    };
}

void Program::startCyclicRegulatoryScenario(const Tests::Cyclic::Regulatory::Params& params)
{
    auto scenario = Tests::ScenarioFactory::createCyclicRegulatory(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::startCyclicShutoffScenario(const Tests::Cyclic::Shutoff::Params& params)
{
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
    qDebug() << params.type;
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

    connect(scenario, &Domain::Tests::AbstractScenario::addPointsRequested,
            this, &Program::addPoints,
            Qt::QueuedConnection);

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

}

void Program::startResponseTest(const Tests::Option::Params& params)
{
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

    auto scenario = Tests::ScenarioFactory::createResolution(
        makeContext(),
        params,
        this
    );

    startScenario(std::move(scenario));
}

void Program::startStepTest(const Tests::Option::Step::Params& params)
{
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

// 991