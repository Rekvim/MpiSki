#pragma once

#include <QObject>
#include <memory>

#include "Domain/Tests/BaseRunner.h"
#include "Domain/Measurement/Sample.h"
#include "Widgets/Chart/ChartType.h"
#include "Storage/Telemetry.h"

#include <QPointF>
#include <QVector>

namespace Domain::Tests {

class AbstractScenario : public QObject
{
    Q_OBJECT

public:
    explicit AbstractScenario(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~AbstractScenario() override = default;

    virtual std::unique_ptr<BaseRunner> createRunner(QObject* parent) = 0;

    virtual void startAnalyzer() {}
    virtual void onSample(const Domain::Measurement::Sample& sample) {}
    virtual void onFinished() {}

signals:
    void telemetryUpdated(const Telemetry& telemetry);

    void pointsRequested(QVector<QVector<QPointF>>& points, Widgets::Chart::ChartType chartType);
    void mainResultUpdated(const Domain::Tests::Main::Result& result);
    void strokeResultUpdated(const Domain::Tests::Stroke::Result& result);
    void stepResultUpdated(const Domain::Tests::Option::Step::Result& result);
    void cyclicRegulatoryResultUpdated(const Domain::Tests::Cyclic::Regulatory::Result& result);
    void cyclicShutoffResultUpdated(const Domain::Tests::Cyclic::Shutoff::Result& result);
    void crossingStatusUpdated(const CrossingStatus& status);

    void addRegressionRequested(const QVector<QPointF>& points);
    void addFrictionRequested(const QVector<QPointF>& points);
    void duplicateMainChartsSeriesRequested();

    void cyclicCycleCompleted(int completedCycles);

    void setMultipleDORequested(const QVector<bool>& states);

    void diRequested(quint8& status);
    void doRequested(quint8& status);
};

}