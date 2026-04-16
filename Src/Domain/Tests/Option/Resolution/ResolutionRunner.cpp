#include "ResolutionRunner.h"
#include "Src/Domain/Program.h"
#include "Src/Storage/Registry.h"
#include "Src/Domain/Tests/Option/OptionTest.h"

RunnerConfig ResolutionRunner::buildConfig()
{
    const auto& p = m_params;

    if (p.points.empty())
        return {};

    auto worker = std::make_unique<OptionTest>();
    OptionTest::Task task;
    task.delay = p.delay;

    const bool normalOpen = isNormallyOpen();

    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    for (auto it = p.points.begin(); it != p.points.end(); ++it)
    {
        const qreal basePercent = normalOpen ? (100.0 - *it) : *it;
        const qreal baseCurrent = 16.0 * basePercent / 100.0 + 4.0;
        const qreal baseRaw = m_mpi.dac()->rawFromValue(baseCurrent);
        task.value.push_back(baseRaw);

        for (auto it_s = p.steps.begin(); it_s < p.steps.end(); ++it_s)
        {
            const qreal stepValue = 16.0 * (*it_s) / 100.0;

            // вверх
            const qreal stepUpCurrent = baseCurrent + stepValue;
            task.value.push_back(m_mpi.dac()->rawFromValue(stepUpCurrent));
            task.value.push_back(baseRaw);

            // вниз
            const qreal stepDownCurrent = baseCurrent - stepValue;
            task.value.push_back(m_mpi.dac()->rawFromValue(stepDownCurrent));
            task.value.push_back(baseRaw);
        }
    }

    task.value.push_back(m_mpi.dac()->rawFromValue(4.0));

    worker->setTask(task);

    const quint64 P = static_cast<quint64>(p.points.size());
    const quint64 S = static_cast<quint64>(p.steps.size());
    const quint64 delay = static_cast<quint64>(p.delay);

    const quint64 N_values =
        2ULL + P * (1ULL + 2ULL * S);

    const quint64 totalMs =
        10000ULL + N_values * delay + 10000ULL;

    return makeConfig(std::move(worker), totalMs, Charts::Resolution);
}

void ResolutionRunner::wireSpecificSignals(Test& base) {
    auto& t = static_cast<OptionTest&>(base);
    auto* owner = qobject_cast<Program*>(parent()); Q_ASSERT(owner);
}
