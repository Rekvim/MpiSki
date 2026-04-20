#include "Program.h"

#include "Src/Domain/Tests/Option/Step/StepAlgorithm.h"
#include "Src/Domain/Tests/Main/MainAlgorithm.h"

#include "Src/Domain/Tests/AnalyzerFactory.h"
#include "Src/Domain/Tests/Stroke/StrokeRunner.h"
#include "Src/Domain/Tests/Main/MainRunner.h"
#include "Src/Domain/Tests/Option/Step/StepRunner.h"
#include "Src/Domain/Tests/Option/Response/ResponseRunner.h"
#include "Src/Domain/Tests/Option/Resolution/ResolutionRunner.h"
#include "Src/Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.h"
#include "Src/Domain/Tests/Cyclic/Shutoff/ShutoffRunner.h"

#include "Src/Domain/DeviceInitializer.h"

#include "Src/Utils/NumberUtils.h"
#include "Src/Utils/SignalUtils.h"
#include <QRegularExpression>
#include <QLocale>
#include <utility>

namespace Domain {
constexpr quint8 VersionFlag = 0x40;

namespace MainTest = Domain::Tests::Main;
namespace StrokeTest = Domain::Tests::Stroke;
namespace StepTest = Domain::Tests::Option::Step;
namespace ResponseTest = Domain::Tests::Option::Response;
namespace ResolutionTest = Domain::Tests::Option::Resolution;
namespace CyclicReg = Domain::Tests::Cyclic::Regulatory;
namespace CyclicShut = Domain::Tests::Cyclic::Shutoff;

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

void Program::setRegistry(Registry *registry)
{
    m_registry = registry;
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

Domain::Measurement::Sample
Program::makeSample() const
{
    Domain::Measurement::Sample s;

    const auto& v = m_registry->valveInfo();
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    s.systemTime = now - m_initTime;
    s.testTime = m_isTestRunning ? now - m_startTime : 0;

    s.dac = m_device.dac()->value();

    s.taskPercent = SignalUtils::calcPercent(
        s.dac,
        v.safePosition == SafePosition::NormallyOpen
    );

    s.diMask = m_device.digitalInputs();
    s.doMask = m_device.digitalOutputs();

    if (auto* linear = m_device.sensorByAdc(0)) {
        s.positionValue = linear->value();
        s.positionPercent = linear->percent();
        s.positionUnit = (v.strokeMovement == StrokeMovement::Rotary) ? "°" : "мм";
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

void Program::updateMainCharts(const Domain::Measurement::Sample& s)
{
    QVector<Point> points;

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

    emit addPoints(Charts::Task, points);

    if (auto* linear = m_device.sensorByAdc(0)) {
        if (!qIsNaN(s.pressure1)) {
            QVector<Point> pressurePoints;
            pressurePoints.push_back({0, s.pressure1, linear->value()});
            emit addPoints(Charts::Pressure, pressurePoints);
        }
    }
}

void Program::updateCyclicChart(const Domain::Measurement::Sample& s, Charts chart)
{
    QVector<Point> points;
    points.push_back({0, qreal(s.testTime), s.taskPercent});
    points.push_back({1, qreal(s.testTime), s.positionPercent});

    emit addPoints(chart, points);

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        quint8 di = m_device.digitalInputs();

        if (di != m_lastDiStatus) {
            QVector<Point> diPts;

            const bool lastClosed = (m_lastDiStatus & 0x01);
            const bool lastOpen = (m_lastDiStatus & 0x02);
            const bool nowClosed  = (di & 0x01);
            const bool nowOpen = (di & 0x02);

            if (nowClosed && !lastClosed) {
                ++m_telemetry.cyclicTestRecord.switch3to0Count;
                diPts.push_back({2, qreal(s.testTime), 0.0});
            } else if (!nowClosed && lastClosed) {
                diPts.push_back({2, qreal(s.testTime), 0.0});
            }

            if (nowOpen && !lastOpen) {
                ++m_telemetry.cyclicTestRecord.switch0to3Count;
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
    const Domain::Measurement::Sample s = makeSample();

    emit sampleReady(s);

    if (m_isTestRunning && m_analyzer) {
        m_analyzer->onSample(s);
    }

    updateRealtimeTexts(s);
    updateChartsFromSample(s);
}

void Program::updateTimeChart(const Domain::Measurement::Sample& s, Charts chart, qint64 time)
{
    QVector<Point> points;

    points.push_back({0, qreal(time), s.taskPercent});
    points.push_back({1, qreal(time), s.positionPercent});

    emit addPoints(chart, points);
}

void Program::updateChartsFromSample(const Domain::Measurement::Sample& s)
{
    updateTimeChart(s, Charts::Trend, s.systemTime);

    switch (m_testWorker)
    {
    case TestWorker::Stroke:
        updateTimeChart(s, Charts::Stroke, s.testTime);
        break;

    case TestWorker::Main:
        updateMainCharts(s);
        break;

    case TestWorker::CyclicRegulatory:
        updateCyclicChart(s, Charts::Cyclic);
        break;

    case TestWorker::CyclicShutOff:
        updateCyclicChart(s, Charts::Cyclic);
        break;

    case TestWorker::Step:
        updateTimeChart(s, Charts::Step, s.testTime);
        break;

    case TestWorker::Response:
        updateTimeChart(s, Charts::Response, s.testTime);
        break;

    case TestWorker::Resolution:
        updateTimeChart(s, Charts::Resolution, s.testTime);
        break;

    default:
        break;
    }
}

void Program::endTest()
{
    m_isTestRunning = false;
    m_testWorker = TestWorker::None;

    emit setTaskControlsEnabled(true);
    emit setButtonInitEnabled(true);

    emit setTask(m_device.dac()->value());

    m_activeRunner.reset();

    m_isCyclicTestRunning = false;
    emit testFinished();
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

    ts.init.initStatusText = "";
    ts.init.connectedSensorsText = "";
    ts.init.deviceStatusText = "";
    ts.init.startingPositionText = "";
    ts.init.finalPositionText = "";

    m_timerSensors->stop();
    m_timerDI->stop();

    emit setButtonInitEnabled(false);

    DeviceInitializer initializer(
        m_device,
        *m_registry,
        m_telemetry
    );

    if (!initializer.connectAndInitDevice()) {
        emit telemetryUpdated(m_telemetry);
        emit setButtonInitEnabled(true);
        return;
    } emit telemetryUpdated(m_telemetry);

    if (!initializer.detectSensors()) {
        emit telemetryUpdated(m_telemetry);
        emit setButtonInitEnabled(true);
        return;
    } emit telemetryUpdated(m_telemetry);

    bool normalClosed =
        m_registry->valveInfo().safePosition == SafePosition::NormallyClosed;

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
            normalClosed,
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetry);

        setDacRaw(0, 10000, true);
        waitForDacCycle();
        initializer.measureStartPositionShutoff(
            normalClosed,
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetry);
    }

    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT) {

        setDacRaw(0, 10000, true);
        waitForDacCycle();
        initializer.measureStartPosition(normalClosed);
        emit telemetryUpdated(m_telemetry);

        setDacRaw(65535, 10000, true);
        waitForDacCycle();
        initializer.measureEndPosition(normalClosed);
        emit telemetryUpdated(m_telemetry);
    }

    initializer.calculateCoefficients();

    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {
        initializer.recordStrokeRange(normalClosed);

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
    emit clearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    quint8 mask = 0;
    for (quint8 adc = 0; adc < 6; ++adc) {
        if (m_device.sensorByAdc(adc))
            mask |= (1 << adc);
    }

    emit setSensorsMask(mask);
    emit setSensorNumber(m_device.sensorCount());
    emit setButtonInitEnabled(true);

    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}

template<typename Runner, typename... Args>
void Program::runTest(Args&&... args)
{
    auto r = std::make_unique<Runner>(
        m_device, *m_registry,
        std::forward<Args>(args)...,
        this
    );

    startRunner(std::move(r));
}

void Program::startMainTest(const MainTest::Params& params)
{
    m_testWorker = TestWorker::Main;
    runTest<MainTest::Runner>(params);
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
    auto &ts = m_telemetry;
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

static void debugCompareMainTest(
    const MainTest::Algorithm::TestResults& oldR,
    const MainTest::Result& newR)
{
    qDebug() << "\n===== MAIN TEST COMPARISON =====";

    qDebug() << "pressureDiff:"
             << "old =" << oldR.pressureDiff
             << "new =" << newR.pressureDiff;

    qDebug() << "friction:"
             << "old =" << oldR.friction
             << "new =" << newR.frictionPercent;

    qDebug() << "dynamicErrorMean:"
             << "old =" << oldR.dynamicErrorMean
             << "new =" << newR.dynamicErrorMean;

    qDebug() << "dynamicErrorMax:"
             << "old =" << oldR.dynamicErrorMax
             << "new =" << newR.dynamicErrorMax;

    qDebug() << "lowLimitPressure:"
             << "old =" << oldR.lowLimitPressure
             << "new =" << newR.lowLimitPressure;

    qDebug() << "highLimitPressure:"
             << "old =" << oldR.highLimitPressure
             << "new =" << newR.highLimitPressure;

    qDebug() << "springLow:"
             << "old =" << oldR.springLow
             << "new =" << newR.springLow;

    qDebug() << "springHigh:"
             << "old =" << oldR.springHigh
             << "new =" << newR.springHigh;

    qDebug() << "linearityError:"
             << "old =" << oldR.linearityError
             << "new =" << newR.linearityError;

    qDebug() << "linearity:"
             << "old =" << oldR.linearity
             << "new =" << newR.linearity;

    qDebug() << "===============================\n";
}

void Program::results_mainTest(const MainTest::Algorithm::TestResults &results)
{
    auto* analyzer = static_cast<MainTest::Analyzer*>(m_analyzer.get());
    analyzer->finish();


    const auto& newResult = analyzer->result();
    debugCompareMainTest(results, newResult);

    const auto& valveInfo = m_registry->valveInfo();
    qreal k = 5 * M_PI * valveInfo.driveDiameter * valveInfo.driveDiameter / 4;

    auto &s = m_telemetry.mainTestRecord;

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
    emit telemetryUpdated(m_telemetry);
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

void Program::startStrokeTest()
{
    m_testWorker = TestWorker::Stroke;

    const auto& valveInfo = m_registry->valveInfo();

    Tests::Stroke::Analyzer::Config cfg;

    cfg.normalClosed = (valveInfo.safePosition == SafePosition::NormallyClosed);

    m_analyzer = AnalyzerFactory::create(m_testWorker);

    auto* analyzer = static_cast<Tests::Stroke::Analyzer*>(m_analyzer.get());

    analyzer->setConfig(cfg);
    analyzer->start();

    runTest<Tests::Stroke::Runner>();
}

void Program::receivedPoints_strokeTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_strokeTest(points, Charts::Stroke);
}

void Program::results_strokeTest()
{
    auto* analyzer = dynamic_cast<Tests::Stroke::Analyzer*>(m_analyzer.get());

    auto result = analyzer->result();
    m_telemetry.stroke = result;

    QString forwardText = QTime(0,0).addMSecs(result.forwardTimeMs).toString("mm:ss.zzz");
    QString backwardText = QTime(0,0).addMSecs(result.backwardTimeMs).toString("mm:ss.zzz");

    m_telemetry.stroke->timeForwardMs = forwardText;
    m_telemetry.stroke->timeBackwardMs = backwardText;

    emit telemetryUpdated(m_telemetry);
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

void Program::receivedPoints_cyclicTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_cyclicTest(points, Charts::Cyclic);
}


void Program::results_cyclicRegulatoryTests()
{
    // auto &dst = m_telemetry.cyclicTestRecord;

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

    // emit telemetryUpdated(m_telemetry);

    auto* analyzer = dynamic_cast<CyclicReg::Analyzer*>(m_analyzer.get());
    analyzer->finish();
    m_telemetry.cyclicTestRecord.regulatoryResult = analyzer->result();

    emit telemetryUpdated(m_telemetry);
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

void Program::results_cyclicShutoffTests(const CyclicShut::Result& results)
{
    auto &dst = m_telemetry.cyclicTestRecord;

    dst.numCyclesShutoff = results.numCycles;
    dst.doOnCounts = results.doOnCounts;
    dst.doOffCounts = results.doOffCounts;

    dst.switch3to0Count = results.switch3to0Count;
    dst.switch0to3Count = results.switch0to3Count;

    emit telemetryUpdated(m_telemetry);
}

void Program::results_cyclicCombinedTests(const CyclicShut::Result& shutoffResults)
{
    // auto &dst = m_telemetry.cyclicTestRecord;
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

    // emit telemetryUpdated(m_telemetry);
}

void Program::prepareRegulatoryTelemetry(const Domain::Tests::Cyclic::Params& params)
{
    auto& rec = m_telemetry.cyclicTestRecord;
    const auto& reg = params.regulatory;
    QStringList parts;
    parts.reserve(reg.sequence.size());

    for (const quint16 v : std::as_const(reg.sequence))
        parts << QString::number(v);

    rec.sequenceRegulatory = parts.join('-');
    rec.numCyclesRegulatory = reg.numCycles;

    const quint64 stepsPerCycle = reg.sequence.size();
    const quint64 totalSteps = stepsPerCycle * reg.numCycles;
    const quint64 totalMs = totalSteps * (reg.delayMs + reg.holdMs);
    rec.totalTimeSecRegulatory = totalMs / 1000.0;
    rec.regulatoryResult = {};
}

void Program::prepareShutoffTelemetry(const Domain::Tests::Cyclic::Params& params)
{
    auto& rec = m_telemetry.cyclicTestRecord;
    const auto& off = params.shutoff;
    QStringList parts;
    parts.reserve(off.sequence.size());

    for (const quint16 v : std::as_const(off.sequence))
        parts << QString::number(v);

    rec.sequenceShutoff = parts.join('-');

    rec.numCyclesShutoff = off.numCycles;

    const quint64 stepsPerCycle = off.sequence.size();
    const quint64 totalSteps = stepsPerCycle * off.numCycles;

    const quint64 totalMs =
        totalSteps * (off.numCycles + off.numCycles);

    rec.totalTimeSecShutoff = totalMs / 1000.0;
}

void Program::runCombinedCyclicTest(const Domain::Tests::Cyclic::Params& params)
{
    const auto& regP = params.regulatory;
    const auto& off = params.shutoff;

    const quint64 regSteps =
        regP.sequence.size() * regP.numCycles;

    const quint64 regMs =
        regSteps * (regP.delayMs + regP.holdMs)
        + regP.delayMs;

    const quint64 offSteps =
        off.sequence.size() * off.numCycles;

    const quint64 offMs =
        offSteps * (off.delayMs + off.holdMs)
        + off.delayMs;

    emit totalTestTimeMs(regMs + offMs);

    auto reg = std::make_unique<CyclicReg::Runner>(
        m_device, *m_registry, params.regulatory, this);

    connect(this, &Program::testFinished,
            this, [this, params]() {
                auto shut = std::make_unique<CyclicShut::Runner>(
                    m_device, *m_registry, params.shutoff, this);

                startRunner(std::move(shut));
            },
            Qt::SingleShotConnection);

    startRunner(std::move(reg));
}

void Program::startCyclicTest(const Domain::Tests::Cyclic::Params& params)
{
    if (params.regulatory.sequence.isEmpty() && params.shutoff.sequence.isEmpty()) {
        emit testFinished();
        return;
    }

    if (params.type == Domain::Tests::Cyclic::Params::Regulatory ||
        params.type == Domain::Tests::Cyclic::Params::Combined) {
        prepareRegulatoryTelemetry(params);
    }

    if (params.type == Domain::Tests::Cyclic::Params::Shutoff) {
        prepareShutoffTelemetry(params);
    }

    switch (params.type)
    {
    case Domain::Tests::Cyclic::Params::Regulatory: {
        m_testWorker = TestWorker::CyclicRegulatory;
        m_analyzer = AnalyzerFactory::create(m_testWorker);

        auto* analyzer = dynamic_cast<CyclicReg::Analyzer*>(m_analyzer.get());
        analyzer->configure(params.regulatory);
        analyzer->start();

        runTest<CyclicReg::Runner>(params.regulatory);
        break;
    } case Domain::Tests::Cyclic::Params::Shutoff: {
        m_testWorker = TestWorker::CyclicShutOff;
        runTest<CyclicShut::Runner>(params.shutoff);
        break;
    } case Domain::Tests::Cyclic::Params::Combined: {
        runCombinedCyclicTest(params);
        break;
    } default:
        break;
    }

    emit telemetryUpdated(m_telemetry);
}

void Program::startResponseTest(const Tests::Option::Params& params) {
    m_testWorker = TestWorker::Response;
    runTest<Tests::Option::Response::Runner>(params);
}

void Program::startResolutionTest(const Tests::Option::Params& params) {
    m_testWorker = TestWorker::Resolution;
    runTest<Tests::Option::Resolution::Runner>(params);
}
void Program::startStepTest(const Tests::Option::Step::Params& params) {
    m_testWorker = TestWorker::Step;

    m_analyzer = AnalyzerFactory::create(m_testWorker);

    auto* analyzer = static_cast<Tests::Option::Step::Analyzer*>(m_analyzer.get());

    analyzer->setConfig({params.testValue});
    analyzer->start();

    runTest<Tests::Option::Step::Runner>(params);
}

void Program::receivedPoints_stepTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_stepTest(points, Charts::Step);
}

void Program::results_stepTest(const QVector<StepTest::Result>& results, quint32 T_value)
{
    auto* analyzer =
        static_cast<Tests::Option::Step::Analyzer*>(m_analyzer.get());

    analyzer->finish();

    const auto& resultsAnalyzer = analyzer->result();

    qDebug() << "\n===== STEP TEST =====";

    for (const auto& r : resultsAnalyzer)
    {
        qDebug() << "from:" << r.from
                 << "to:" << r.to
                 << "T:" << r.T_value
                 << "overshoot:" << r.overshoot;
    }

    qDebug() << "=====================\n";

    m_telemetry.stepResults.clear();
    for (auto &r : results) {
        StepTestRecord rec;
        rec.from = r.from;
        rec.to = r.to;
        rec.T_value = r.T_value;
        rec.overshoot = r.overshoot;
        m_telemetry.stepResults.push_back(rec);
    }

    emit telemetryUpdated(m_telemetry);

    emit setStepResults(results, T_value);
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