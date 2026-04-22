#include "Runner.h"
#include "Src/Domain/Tests/Main/Algorithm.h"
#include "Src/Domain/Program.h"

namespace Domain::Tests::Main {
    RunnerConfig Runner::buildConfig() {
        auto p = m_params;
        if (p.delay == 0) return {};

        const quint64 N = static_cast<quint64>(p.pointNumbers);
        const quint64 upMs = p.delay + N * p.response;
        const quint64 downMs = p.delay + N * p.response;
        const quint64 totalMs = 10000ULL + upMs + downMs + 10000ULL;

        p.dac_min = qMax(m_device.dac()->rawFromValue(p.signal_min), m_device.dacMin());
        p.dac_max = qMin(m_device.dac()->rawFromValue(p.signal_max), m_device.dacMax());

        auto worker = std::make_unique<Algorithm>();
        worker->setParameters(p);

        return makeConfig(std::move(worker), totalMs, Widgets::Chart::ChartType::Task);
    }

    void Runner::wireSpecificSignals(Test& base) {
        auto& t = static_cast<Algorithm&>(base);
        auto owner = qobject_cast<Program*>(parent());

        connect(&t, &Algorithm::getPoints,
                owner, &Program::receivedPoints_mainTest,
                Qt::BlockingQueuedConnection);

        connect(&t, &Algorithm::dublSeries,
                owner, [owner]{ emit owner->duplicateMainChartsSeries(); });

        connect(&t, &Algorithm::addRegression,
                owner, &Program::addRegression,
                Qt::QueuedConnection);

        connect(&t, &Algorithm::addFriction,
                owner, &Program::addFriction,
                Qt::QueuedConnection);

        connect(&t, &Algorithm::results,
                owner, &Program::results_mainTest,
                Qt::QueuedConnection);

        // connect(&t, &MainTest::clearGraph, owner, [owner]{
        //     emit owner->clearPoints(Charts::Task);
        //     emit owner->clearPoints(Charts::Pressure);
        //     emit owner->clearPoints(Charts::Friction);
        //     emit owner->setRegressionEnable(false);
        // });
    }
}