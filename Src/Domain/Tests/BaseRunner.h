#pragma once

#include <QThread>
#include <QObject>

#include "Test.h"
#include "Src/CustomChart/ChartManager.h"
#include "Src/Storage/Registry.h"

class Mpi;

struct RunnerConfig {
    std::unique_ptr<Test> worker = nullptr;
    quint64 totalMs = 0;
    Charts chartToClear = Charts::None;
};

class BaseRunner : public QObject {
    Q_OBJECT
public:
    BaseRunner(Mpi& mpi, Registry& reg, QObject* parent=nullptr);
    ~BaseRunner();

public slots:
    void start();
    void stop();
    void releaseBlock();

signals:
    void requestClearChart(Charts chartIndex);
    void totalTestTimeMs(quint64);
    void endTest();
    void testActuallyStarted();
    void requestSetDAC(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart);

protected:
    virtual RunnerConfig buildConfig() = 0;
    virtual void wireSpecificSignals(Test& t) {}

protected:

    bool isNormallyOpen() const {
        return m_reg.valveInfo().safePosition == SafePosition::NormallyOpen;
    }

    RunnerConfig makeConfig(std::unique_ptr<Test> worker,
                            quint64 totalMs,
                            Charts chart) {
        RunnerConfig cfg;
        cfg.worker = std::move(worker);
        cfg.totalMs = totalMs;
        cfg.chartToClear = chart;
        return cfg;
    }

    Mpi& m_mpi;
    Registry& m_reg;

private:
    QThread* m_thread = nullptr;
    std::unique_ptr<Test> m_worker = nullptr;
};
