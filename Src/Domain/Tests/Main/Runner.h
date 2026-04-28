#pragma once

#include "Domain/Tests/BaseRunner.h"
#include "Domain/Tests/Main/Params.h"
#include "Algorithm.h"

namespace Domain::Tests::Main {

class Runner : public BaseRunner {
    Q_OBJECT
public:
    Runner(Mpi::Device& device, bool normalOpen, const Params& params, QObject* parent = nullptr)
        : BaseRunner(device, normalOpen, parent), m_params(params) {}
signals:
    void dublSeries();
    void points(QVector<QVector<QPointF>> &points);
    void addRegression(const QVector<QPointF> &points);
    void addFriction(const QVector<QPointF> &points);
    void results(const Domain::Tests::Main::Algorithm::TestResults& results);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(AbstractTestAlgorithm& t) override;

private:
    Params m_params;
};
}
