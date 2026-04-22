#pragma once

#include <QThread>
#include <QObject>
#include <memory>

#include "Test.h"
#include "Src/Widgets/Chart/Manager.h"
#include "Src/Storage/Registry.h"
#include "Src/Domain/Mpi/Device.h"

struct RunnerConfig {
    std::unique_ptr<Test> worker = nullptr;
    quint64 totalMs = 0;
    Widgets::Chart::ChartType chartToClear = Widgets::Chart::ChartType::None;
};

class BaseRunner : public QObject
{
    Q_OBJECT

public:
    BaseRunner(Domain::Mpi::Device& device, Registry& reg, QObject* parent = nullptr);
    ~BaseRunner() override;

public slots:
    void start();
    void stop();
    void releaseBlock();

signals:
    void requestClearChart(Widgets::Chart::ChartType chartIndex);
    void totalTestTimeMs(quint64 totalMs);
    void endTest();
    void testActuallyStarted();
    void requestSetDAC(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart);

protected:
    virtual RunnerConfig buildConfig() = 0;
    virtual void wireSpecificSignals(Test& t) = 0;

protected:
    bool isNormallyOpen() const
    {
        return m_reg.valveInfo().safePosition == SafePosition::NormallyOpen;
    }

    RunnerConfig makeConfig(std::unique_ptr<Test> worker,
                            quint64 totalMs,
                            Widgets::Chart::ChartType chart)
    {
        RunnerConfig cfg;
        cfg.worker = std::move(worker);
        cfg.totalMs = totalMs;
        cfg.chartToClear = chart;
        return cfg;
    }

    Domain::Mpi::Device& m_device;
    Registry& m_reg;

private:
    void cleanupThread();

private:
    QThread* m_thread = nullptr;
    Test* m_worker = nullptr;
};