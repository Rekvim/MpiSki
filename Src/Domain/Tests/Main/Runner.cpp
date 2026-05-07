#include "Runner.h"

#include "Domain/Measurement/Sensor.h"
#include "Domain/Mpi/Device.h"

namespace Domain::Tests::Main {

RunnerConfig Runner::buildConfig()
{
    auto p = m_params;

    if (p.delay == 0)
        return {};

    const quint64 N = static_cast<quint64>(p.pointNumbers);

    const quint64 upMs = p.delay + N * p.response;
    const quint64 downMs = p.delay + N * p.response;
    const quint64 totalMs = 10000ULL + upMs + downMs + 10000ULL;

    p.dac_min =
        qMax(m_device.dac()->rawFromValue(p.signal_min),
             m_device.dacMin());

    p.dac_max =
        qMin(m_device.dac()->rawFromValue(p.signal_max),
             m_device.dacMax());

    auto worker = std::make_unique<Algorithm>();
    worker->setParameters(p);

    return makeConfig(
        std::move(worker),
        totalMs,
        Widgets::Chart::ChartType::Task
        );
}

void Runner::wireSpecificSignals(AbstractTestAlgorithm& base)
{
    auto& t = static_cast<Algorithm&>(base);

    connect(&t, &Algorithm::backwardStrokeStarted,
            this, &Runner::startBackwardStroke,
            Qt::QueuedConnection);

    connect(&t, &Algorithm::dublSeries,
            this, &Runner::dublSeries,
            Qt::QueuedConnection);

    connect(&t, &Algorithm::processCompleted,
            this, &Runner::processCompleted,
            Qt::QueuedConnection);
}

}