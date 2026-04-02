#include "Program.h"

#include "Src/Tests/StepTest.h"
#include "Src/Tests/StrokeTest.h"
#include "Src/Tests/MainTest.h"

#include "Src/Runners/MainTestRunner.h"
#include "Src/Runners/StepTestRunner.h"
#include "Src/Runners/StrokeTestRunner.h"
#include "Src/Runners/OptionResponseRunner.h"
#include "Src/Runners/OptionResolutionRunner.h"
#include "Src/Runners/CyclicRegulatoryRunner.h"
#include "Src/Runners/CyclicShutoffRunner.h"

#include "Src/Domain/DeviceInitializer.h"

#include "Src/Utils/NumberUtils.h"
#include "Src/Utils/SignalUtils.h"


#include <QRegularExpression>
#include <QLocale>
#include <utility>

constexpr quint8 VersionFlag = 0x40;

Program::Program(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<RealtimeState>("RealtimeState");
    qRegisterMetaType<Sample>("Sample");

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

Sample Program::makeSample() const
{
    Sample s;

    const auto& v = m_registry->valveInfo();

    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    s.systemTime = now - m_initTime;
    s.testTime = m_isTestRunning ? now - m_startTime : 0;

    s.dac = m_mpi.dac()->value();

    s.taskPercent = SignalUtils::calcPercent(
        s.dac,
        v.safePosition == SafePosition::NormallyOpen
        );

    if (auto* linear = m_mpi.sensorByAdc(0)) {
        s.positionValue = linear->value();
        s.positionPercent = linear->percent();
        s.positionUnit = (v.strokeMovement == StrokeMovement::Rotary) ? "°" : "мм";
    }

    if (auto* p1 = m_mpi.sensorByAdc(1))
        s.pressure1 = p1->value();

    if (auto* p2 = m_mpi.sensorByAdc(2))
        s.pressure2 = p2->value();

    if (auto* p3 = m_mpi.sensorByAdc(3))
        s.pressure3 = p3->value();

    if (auto* fb = m_mpi.sensorByAdc(4))
        s.feedbackCurrent = fb->value();

    return s;
}

void Program::updateRealtimeTexts(const Sample& s)
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

void Program::updateTrendChart(const Sample& s)
{
    QVector<Point> points;

    points.push_back({0, qreal(s.systemTime), s.taskPercent});
    points.push_back({1, qreal(s.systemTime), s.positionPercent});

    emit addPoints(Charts::Trend, points);
}

void Program::updateStrokeChart(const Sample& s)
{
    QVector<Point> points;

    points.push_back({0, qreal(s.testTime), s.taskPercent});
    points.push_back({1, qreal(s.testTime), s.positionPercent});

    emit addPoints(Charts::Stroke, points);
}

void Program::updateMainCharts(const Sample& s)
{
    QVector<Point> points;

    if (auto* linear = m_mpi.sensorByAdc(0)) {
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

    if (auto* linear = m_mpi.sensorByAdc(0)) {
        if (!qIsNaN(s.pressure1)) {
            QVector<Point> pressurePoints;
            pressurePoints.push_back({0, s.pressure1, linear->value()});
            emit addPoints(Charts::Pressure, pressurePoints);
        }
    }
}

void Program::updateOptionChart(const Sample& s, Charts chart)
{
    QVector<Point> points;
    points.push_back({0, qreal(s.testTime), s.taskPercent});
    points.push_back({1, qreal(s.testTime), s.positionPercent});

    emit addPoints(chart, points);
}

void Program::updateCyclicChart(const Sample& s)
{
    QVector<Point> points;
    points.push_back({0, qreal(s.testTime), s.taskPercent});
    points.push_back({1, qreal(s.testTime), s.positionPercent});

    emit addPoints(Charts::Cyclic, points);

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        quint8 di = m_mpi.digitalInputs();

        if (di != m_lastDiStatus) {
            QVector<Point> diPts;

            const bool lastClosed = (m_lastDiStatus & 0x01);
            const bool lastOpen   = (m_lastDiStatus & 0x02);
            const bool nowClosed  = (di & 0x01);
            const bool nowOpen    = (di & 0x02);

            if (nowClosed && !lastClosed) {
                ++m_telemetryStore.cyclicTestRecord.switch3to0Count;
                diPts.push_back({2, qreal(s.testTime), 0.0});
            } else if (!nowClosed && lastClosed) {
                diPts.push_back({2, qreal(s.testTime), 0.0});
            }

            if (nowOpen && !lastOpen) {
                ++m_telemetryStore.cyclicTestRecord.switch0to3Count;
                diPts.push_back({3, qreal(s.testTime), 100.0});
            } else if (!nowOpen && lastOpen) {
                diPts.push_back({3, qreal(s.testTime), 100.0});
            }

            if (!diPts.isEmpty())
                emit addPoints(Charts::Cyclic, diPts);

            m_lastDiStatus = di;
        }
    }
}

void Program::updateSensors()
{
    const Sample s = makeSample();

    emit sampleReady(s);

    if (m_isTestRunning) {
        m_testDataBuffer.add(s);
        m_strokeAnalyzer.onSample(s);
    }

    updateRealtimeTexts(s);
    updateChartsFromSample(s);
}

void Program::updateChartsFromSample(const Sample& s)
{
    updateTrendChart(s);

    switch (m_activeChartMode)
    {
    case ActiveChartMode::Stroke:
        updateStrokeChart(s);
        break;

    case ActiveChartMode::Main:
        updateMainCharts(s);
        break;

    case ActiveChartMode::Cyclic:
        updateCyclicChart(s);
        break;

    case ActiveChartMode::Step:
        updateOptionChart(s, Charts::Step);
        break;

    case ActiveChartMode::Response:
        updateOptionChart(s, Charts::Response);
        break;

    case ActiveChartMode::Resolution:
        updateOptionChart(s, Charts::Resolution);
        break;

    default:
        break;
    }
}

void Program::endTest()
{
    m_isTestRunning = false;
    m_activeChartMode = ActiveChartMode::TrendOnly;

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

void Program::setDacReal(qreal value)
{
    m_mpi.setDacValue(value);
}

void Program::setInitDoStates(const QVector<bool> &states)
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

    DeviceInitializer initializer(
        m_mpi,
        *m_registry,
        m_telemetryStore
    );

    if (!initializer.connectAndInitDevice()) {
        emit telemetryUpdated(m_telemetryStore);
        emit setButtonInitEnabled(true);
        return;
    } emit telemetryUpdated(m_telemetryStore);

    if (!initializer.detectSensors()) {
        emit telemetryUpdated(m_telemetryStore);
        emit setButtonInitEnabled(true);
        return;
    } emit telemetryUpdated(m_telemetryStore);

    bool normalClosed =
        m_registry->valveInfo().safePosition == SafePosition::NormallyClosed;

    if (m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT ||
        m_patternType == SelectTests::Pattern_C_SOVT) {

        if ((m_mpi.version() & VersionFlag) != 0) {
            emit setDoButtonsChecked(m_mpi.digitalOutputs());
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

        emit telemetryUpdated(m_telemetryStore);

        setDacRaw(0, 10000, true);
        waitForDacCycle();
        initializer.measureStartPositionShutoff(
            normalClosed,
            m_initialDoStates,
            m_savedInitialDoStates);

        emit telemetryUpdated(m_telemetryStore);
    }

    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT) {

        setDacRaw(0, 10000, true);
        waitForDacCycle();
        initializer.measureStartPosition(normalClosed);
        emit telemetryUpdated(m_telemetryStore);

        setDacRaw(65535, 10000, true);
        waitForDacCycle();
        initializer.measureEndPosition(normalClosed);
        emit telemetryUpdated(m_telemetryStore);
    }

    initializer.calculateCoefficients();

    if (m_patternType == SelectTests::Pattern_B_CVT ||
        m_patternType == SelectTests::Pattern_C_CVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {
        initializer.recordStrokeRange(normalClosed);

        setDacRaw(0, 10000, true);

        emit telemetryUpdated(m_telemetryStore);
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

template<typename Runner, typename... Args>
void Program::runTest(Args&&... args)
{
    auto r = std::make_unique<Runner>(
        m_mpi,
        *m_registry,
        std::forward<Args>(args)...,
        this
        );

    startRunner(std::move(r));
}

void Program::startMainTest(const MainTestSettings::TestParameters& params)
{
    m_activeChartMode = ActiveChartMode::Main;
    runTest<MainTestRunner>(params);
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
    m_activeChartMode = ActiveChartMode::Stroke;
    const auto& valveInfo = m_registry->valveInfo();

    StrokeTestAnalyzer::Config cfg;
    cfg.normalClosed =
        (valveInfo.safePosition == SafePosition::NormallyClosed);

    m_strokeAnalyzer.setConfig(cfg);
    m_strokeAnalyzer.start();

    runTest<StrokeTestRunner>();
}

void Program::receivedPoints_strokeTest(QVector<QVector<QPointF>> &points)
{
    emit getPoints_strokeTest(points, Charts::Stroke);
}

void Program::results_strokeTest()
{
    auto result = m_strokeAnalyzer.finish();

    QString forwardText =
        QTime(0,0).addMSecs(result.forwardTimeMs).toString("mm:ss.zzz");

    QString backwardText =
        QTime(0,0).addMSecs(result.backwardTimeMs).toString("mm:ss.zzz");

    m_telemetryStore.strokeTestRecord.timeForwardMs = forwardText;
    m_telemetryStore.strokeTestRecord.timeBackwardMs = backwardText;

    emit telemetryUpdated(m_telemetryStore);
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

void Program::prepareRegulatoryTelemetry(
    const CyclicTestSettings::TestParameters& params)
{
    auto& rec = m_telemetryStore.cyclicTestRecord;

    QStringList parts;
    parts.reserve(params.regSeqValues.size());

    for (const quint16 v : std::as_const(params.regSeqValues))
        parts << QString::number(v);

    rec.sequenceRegulatory = parts.join('-');

    rec.numCyclesRegulatory = params.regulatory_numCycles;

    const quint64 stepsPerCycle = params.regSeqValues.size();
    const quint64 totalSteps = stepsPerCycle * params.regulatory_numCycles;

    const quint64 totalMs =
        totalSteps * (params.regulatory_delayMs + params.regulatory_holdMs);

    rec.totalTimeSecRegulatory = totalMs / 1000.0;

    rec.ranges.clear();
    rec.ranges.resize(params.regSeqValues.size());

    for (int i = 0; i < params.regSeqValues.size(); ++i) {

        auto& r = rec.ranges[i];

        r.rangePercent = params.regSeqValues[i];

        r.maxForwardValue = std::numeric_limits<qreal>::lowest();
        r.maxForwardCycle = -1;

        r.maxReverseValue = std::numeric_limits<qreal>::max();
        r.maxReverseCycle = -1;
    }
}

void Program::prepareShutoffTelemetry(
    const CyclicTestSettings::TestParameters& params)
{
    auto& rec = m_telemetryStore.cyclicTestRecord;

    QStringList parts;
    parts.reserve(params.offSeqValues.size());

    for (const quint16 v : std::as_const(params.offSeqValues))
        parts << QString::number(v);

    rec.sequenceShutoff = parts.join('-');

    rec.numCyclesShutoff = params.shutoff_numCycles;

    const quint64 stepsPerCycle = params.offSeqValues.size();
    const quint64 totalSteps = stepsPerCycle * params.shutoff_numCycles;

    const quint64 totalMs =
        totalSteps * (params.shutoff_delayMs + params.shutoff_holdMs);

    rec.totalTimeSecShutoff = totalMs / 1000.0;
}

void Program::runCombinedCyclicTest(
    const CyclicTestSettings::TestParameters& params)
{
    const quint64 regSteps =
        params.regSeqValues.size() * params.regulatory_numCycles;

    const quint64 regMs =
        regSteps * (params.regulatory_delayMs + params.regulatory_holdMs)
        + params.regulatory_delayMs;

    const quint64 offSteps =
        params.offSeqValues.size() * params.shutoff_numCycles;

    const quint64 offMs =
        offSteps * (params.shutoff_delayMs + params.shutoff_holdMs)
        + params.shutoff_delayMs;

    emit totalTestTimeMs(regMs + offMs);

    auto reg = std::make_unique<CyclicRegulatoryRunner>(
        m_mpi, *m_registry, params, this);

    connect(this, &Program::testFinished,
            this,
            [this, params]()
            {
                auto shut = std::make_unique<CyclicShutoffRunner>(
                    m_mpi, *m_registry, params, this);

                startRunner(std::move(shut));
            },
            Qt::SingleShotConnection);

    startRunner(std::move(reg));
}

void Program::startCyclicTest(const CyclicTestSettings::TestParameters& params)
{
    if (params.regSeqValues.isEmpty() && params.offSeqValues.isEmpty()) {
        emit testFinished();
        return;
    }

    if (params.testType == CyclicTestSettings::TestParameters::Regulatory ||
        params.testType == CyclicTestSettings::TestParameters::Combined) {

        prepareRegulatoryTelemetry(params);
    }

    if (params.testType == CyclicTestSettings::TestParameters::Shutoff) {
        prepareShutoffTelemetry(params);
    }

    m_activeChartMode = ActiveChartMode::Cyclic;

    switch (params.testType)
    {
    case CyclicTestSettings::TestParameters::Regulatory:
        runTest<CyclicRegulatoryRunner>(params);
        break;
    case CyclicTestSettings::TestParameters::Shutoff:
        runTest<CyclicShutoffRunner>(params);
        break;
    case CyclicTestSettings::TestParameters::Combined:
        runCombinedCyclicTest(params);
        break;
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

void Program::startResponseTest(const OtherTestSettings::TestParameters& params) {
    m_activeChartMode = ActiveChartMode::Response;
    runTest<OptionResponseRunner>(params);
}

void Program::startResolutionTest(const OtherTestSettings::TestParameters& params) {
    m_activeChartMode = ActiveChartMode::Resolution;
    runTest<OptionResponseRunner>(params);
}
void Program::startStepTest(const StepTestSettings::TestParameters& params) {
    m_activeChartMode = ActiveChartMode::Step;
    runTest<StepTestRunner>(params);
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
