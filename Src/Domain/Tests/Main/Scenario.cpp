#include "Scenario.h"

#include "Runner.h"
#include "Analyzer.h"

#include "Utils/NumberUtils.h"

#include <QDebug>
#include <QtMath>

namespace Domain::Tests::Main {

namespace {

static bool inRange(double value, double lower, double upper)
{
    if (lower > upper)
        std::swap(lower, upper);

    return value >= lower && value <= upper;
}

static void debugCompareMainTest(
    const Result& algorithmResult,
    const Result& analyzerResult)
{
    qDebug() << "\n Результаты основного теста";

    qDebug() << "pressureDiff:"
             << "algorithm =" << algorithmResult.pressureDiff
             << "analyzer =" << analyzerResult.pressureDiff;

    qDebug() << "frictionForce:"
             << "algorithm =" << algorithmResult.frictionForce
             << "analyzer =" << analyzerResult.frictionForce;

    qDebug() << "frictionPercent:"
             << "algorithm =" << algorithmResult.frictionPercent
             << "analyzer =" << analyzerResult.frictionPercent;

    qDebug() << "dynamicErrorMean:"
             << "algorithm =" << algorithmResult.dynamicErrorMean
             << "analyzer =" << analyzerResult.dynamicErrorMean;

    qDebug() << "dynamicErrorMeanPercent:"
             << "algorithm =" << algorithmResult.dynamicErrorMeanPercent
             << "analyzer =" << analyzerResult.dynamicErrorMeanPercent;

    qDebug() << "dynamicErrorMax:"
             << "algorithm =" << algorithmResult.dynamicErrorMax
             << "analyzer =" << analyzerResult.dynamicErrorMax;

    qDebug() << "dynamicErrorMaxPercent:"
             << "algorithm =" << algorithmResult.dynamicErrorMaxPercent
             << "analyzer =" << analyzerResult.dynamicErrorMaxPercent;

    qDebug() << "dynamicErrorReal:"
             << "algorithm =" << algorithmResult.dynamicErrorReal
             << "analyzer =" << analyzerResult.dynamicErrorReal;

    qDebug() << "lowLimitPressure:"
             << "algorithm =" << algorithmResult.lowLimitPressure
             << "analyzer =" << analyzerResult.lowLimitPressure;

    qDebug() << "highLimitPressure:"
             << "algorithm =" << algorithmResult.highLimitPressure
             << "analyzer =" << analyzerResult.highLimitPressure;

    qDebug() << "springLow:"
             << "algorithm =" << algorithmResult.springLow
             << "analyzer =" << analyzerResult.springLow;

    qDebug() << "springHigh:"
             << "algorithm =" << algorithmResult.springHigh
             << "analyzer =" << analyzerResult.springHigh;

    qDebug() << "linearityError:"
             << "algorithm =" << algorithmResult.linearityError
             << "analyzer =" << analyzerResult.linearityError;

    qDebug() << "linearity:"
             << "algorithm =" << algorithmResult.linearity
             << "analyzer =" << analyzerResult.linearity;

    qDebug() << "===============================\n";
}

}

Scenario::Scenario(Tests::TestContext context,
                   const Params& params,
                   QObject* parent)
    : Tests::TestScenario(parent)
    , m_context(context)
    , m_params(params)
{
}

Scenario::~Scenario() = default;

void Scenario::startAnalyzer()
{
    m_analyzer = std::make_unique<Analyzer>();

    Analyzer::Config cfg;
    cfg.driveDiameter = m_context.config.driveDiameter;

    m_analyzer->setConfig(cfg);
    m_analyzer->start();
}

void Scenario::onSample(const Measurement::Sample& sample)
{
    if (m_analyzer)
        m_analyzer->onSample(sample);
}

std::unique_ptr<BaseRunner> Scenario::createRunner(QObject* parent)
{
    const bool normalOpen =
        m_context.config.safePosition == SafePosition::NormallyOpen;

    auto runner = std::make_unique<Runner>(
        m_context.device,
        normalOpen,
        m_params,
        parent
    );

    connect(runner.get(), &Runner::dublSeries,
            this, &Scenario::duplicateMainChartsSeriesRequested,
            Qt::QueuedConnection);

    connect(runner.get(), &Runner::addRegression,
            this, &Scenario::addRegressionRequested,
            Qt::QueuedConnection);

    connect(runner.get(), &Runner::addFriction,
            this, &Scenario::addFrictionRequested,
            Qt::QueuedConnection);

    connect(runner.get(), &Runner::points,
            this, [this](QVector<QVector<QPointF>>& points) {
                emit pointsRequested(
                    points,
                    Widgets::Chart::ChartType::Task
                    );
            },
            Qt::BlockingQueuedConnection);

    return runner;
}

void Scenario::onResults(const Algorithm::TestResults& results)
{
    auto& main = m_context.telemetry.testMain.emplace();

    const qreal k =
        5 * M_PI * m_context.config.driveDiameter * m_context.config.driveDiameter / 4;

    main.pressureDiff = results.pressureDiff;

    main.frictionForce = results.pressureDiff * k;
    main.frictionPercent = results.friction;

    main.dynamicErrorMean = results.dynamicErrorMean;
    main.dynamicErrorMeanPercent = results.dynamicErrorMean / 0.16;

    main.dynamicErrorMax = results.dynamicErrorMax;
    main.dynamicErrorMaxPercent = results.dynamicErrorMax / 0.16;

    main.dynamicErrorReal = results.dynamicErrorMean / 0.16;

    main.lowLimitPressure = results.lowLimitPressure;
    main.highLimitPressure = results.highLimitPressure;

    main.springLow = results.springLow;
    main.springHigh = results.springHigh;

    main.linearityError = results.linearityError;
    main.linearity = results.linearity;

    if (m_analyzer) {
        m_analyzer->finish();

        const auto& analyzerResult = m_analyzer->result();

        debugCompareMainTest(*m_context.telemetry.testMain, analyzerResult);
    }

    updateCrossingStatus();

    emit telemetryUpdated(m_context.telemetry);
}

void Scenario::updateCrossingStatus()
{
    auto& ts = m_context.telemetry;
    const auto& cfg = m_context.config;
    const QString& valveStroke = cfg.valveStroke;
    const auto& limits = cfg.crossingLimits;

    using State = CrossingStatus::State;

    if (!ts.testMain)
        return;

    if (limits.frictionEnabled) {
        ts.crossingStatus.frictionPercent =
            inRange(ts.testMain->frictionPercent,
                    limits.frictionCoefLower,
                    limits.frictionCoefUpper)
                ? State::Ok
                : State::Fail;
    } else {
        ts.crossingStatus.frictionPercent = State::Unknown;
    }

    if (limits.valveStrokeEnabled) {
        bool ok = false;
        const double recStroke = NumberUtils::toDouble(valveStroke, &ok);

        if (ok) {
            const double d =
                std::abs(recStroke) * (limits.valveStroke / 100.0);

            const double lo = recStroke - d;
            const double hi = recStroke + d;

            ts.crossingStatus.valveStroke =
                inRange(ts.valveStrokeRecord.real, lo, hi)
                    ? State::Ok
                    : State::Fail;
        } else {
            ts.crossingStatus.valveStroke = State::Unknown;
        }
    } else {
        ts.crossingStatus.valveStroke = State::Unknown;
    }

    if (limits.dynamicErrorEnabled) {
        ts.crossingStatus.dynamicError =
            inRange(ts.testMain->dynamicErrorReal,
                    0.0,
                    cfg.dinamicErrorRecomend)
                ? State::Ok
                : State::Fail;
    } else {
        ts.crossingStatus.dynamicError = State::Unknown;
    }

    if (limits.springEnabled) {
        double recLow = cfg.driveRangeLow;
        double recHigh = cfg.driveRangeHigh;

        if (recLow > recHigh)
            std::swap(recLow, recHigh);

        const double lowD = std::abs(recLow) * (limits.springLower / 100.0);
        const double highD = std::abs(recHigh) * (limits.springUpper / 100.0);

        const double lowLo = recLow - lowD;
        const double lowHi = recLow + lowD;

        const double highLo = recHigh - highD;
        const double highHi = recHigh + highD;

        const bool okLow =
            inRange(ts.testMain->springLow, lowLo, lowHi);

        const bool okHigh =
            inRange(ts.testMain->springHigh, highLo, highHi);

        ts.crossingStatus.spring =
            (okLow && okHigh) ? State::Ok : State::Fail;
    } else {
        ts.crossingStatus.spring = State::Unknown;
    }

    if (limits.linearCharacteristicEnabled) {
        ts.crossingStatus.linearCharacteristic =
            inRange(ts.testMain->linearityError,
                    0.0,
                    limits.linearCharacteristic)
                ? State::Ok
                : State::Fail;
    } else {
        ts.crossingStatus.linearCharacteristic = State::Unknown;
    }
}

}