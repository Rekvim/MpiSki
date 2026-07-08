#pragma once

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "Domain/Mpi/Device.h"

#include "Widgets/Chart/ChartType.h"
#include "Widgets/Chart/Point.h"


#include "Storage/Telemetry.h"

#include "Domain/Measurement/Sample.h"
#include "Domain/Measurement/TestDataBuffer.h"
#include "Domain/Tests/BaseRunner.h"
#include "Domain/Tests/Main/Result.h"

#include "Domain/DeviceProfile.h"
#include "DeviceConfig.h"

namespace Domain::Tests {
class AbstractScenario;
}

namespace Domain::Tests {
class Context;
}
namespace Domain::Tests::Cyclic {
struct Params;
}

namespace Domain::Tests::Cyclic::Regulatory {
struct Params;
struct Result;
}

namespace Domain::Tests::Cyclic::Shutoff {
struct Result;
struct Params;
}


namespace Domain::Tests::Main {
struct Params;
}

namespace Domain::Tests::Option {
struct Params;
}

namespace Domain::Tests::Option::Step {
struct Params;
struct Result;
}

namespace Widgets::Chart {
struct Point;
}

namespace Domain {
class Program : public QObject
{
    Q_OBJECT
public:

    explicit Program(QObject *parent = nullptr);

    void setConfig(const Domain::DeviceConfig& deviceConfig) { m_deviceConfig = deviceConfig; }

signals:
    void sampleReady(const Domain::Measurement::Sample& sample);
    void testStartRejected(const QString& reason);
    void telemetryUpdated(const Telemetry& telemetry);

    void errorOccured(const QString& error);

    void cyclicCycleCompleted(int completedCycles);

    void testActuallyStarted();

    void setSensorsMask(quint8 adcMask);

    void setTask(qreal task);
    void setSensorNumber(quint8 num);
    void setGroupDOVisible(bool visible);
    void setVisible(Widgets::Chart::ChartType chart, quint16 series, bool visible);
    void setRegressionEnable(bool enable);

    void mainResultUpdated(const Domain::Tests::Main::Result& result);
    void strokeResultUpdated(const Domain::Tests::Stroke::Result& result);
    void stepResultUpdated(const Domain::Tests::Option::Step::Result& result);
    void cyclicRegulatoryResultUpdated(const Domain::Tests::Cyclic::Regulatory::Result& result);
    void cyclicShutoffResultUpdated(const Domain::Tests::Cyclic::Shutoff::Result& result);
    void crossingStatusUpdated(const CrossingStatus& status);

    void setDoButtonsChecked(quint8 status);

    void setDiCheckboxesChecked(quint8 status);

    void addPoints(Widgets::Chart::ChartType chartType, const QVector<Widgets::Chart::Point>& points);
    void clearPoints(Widgets::Chart::ChartType chartType);

    void stopTheTest();
    void duplicateMainChartsSeries();
    void releaseBlock();

    bool question(QString& title, QString& text);

    void testFinished();

    void totalTestTimeMs(quint64 totalMs);
    void runnerFinished();

private:
    bool isDeviceReadyForTest() const;
    void failToStartTest(const QString& reason);
    DeviceConfig m_deviceConfig;
    Domain::DeviceProfile m_deviceProfile;

    // Sample
    Domain::Measurement::Sample makeSample() const;
    Domain::Measurement::TestDataBuffer m_testDataBuffer;

    std::unique_ptr<Domain::Tests::AbstractScenario> m_currentScenario;
    Domain::Tests::Main::Result m_lastMainResult;

    void recomputeMainResult(Domain::Tests::Main::Result& r) const;
    //
    void onRunnerActuallyStarted();

    // init
    void waitForDacCycle();
    void finalizeInitialization();

    bool m_suppressPublicTestFinished = false;

    void startScenario(std::unique_ptr<Domain::Tests::AbstractScenario> scenario);
    void connectScenarioRuntime(Domain::Tests::AbstractScenario* scenario);

    Tests::Context makeContext();
    void startCyclicRegulatoryScenario(const Tests::Cyclic::Regulatory::Params& params);
    void startCyclicShutoffScenario(const Tests::Cyclic::Shutoff::Params& params);

    Registry* m_registry;

    Domain::Mpi::Device m_device;

    Telemetry m_telemetry;
    QTimer* m_diPollTimer = nullptr;
    QTimer* m_timerSensors;
    QTimer* m_timerDI;

    quint64 m_cyclicStartTimeMs = 0;
    quint64 m_startTime;
    quint64 m_initTime;
    QEventLoop *m_dacEventLoop;

    bool m_isInitialized = false;
    bool m_isCyclicTestRunning = false;
    bool m_isTestRunning = false;
    bool m_isDacStopRequested;
    bool m_shouldWaitForButton = false;
    QVector<bool> m_initialDoStates;
    QVector<bool> m_savedInitialDoStates;

private slots:
    void updateSensors();

public slots:
    void setMultipleDO(const QVector<bool>& states);

    void setDacRaw(quint16 dac,
                quint32 sleep_ms = 0,
                bool waitForStop = false,
                bool waitForStart = false);

    void initialization();

    void setInitDoStates(const QVector<bool>& states);
    void setProfile(const Domain::DeviceProfile& profile) { m_deviceProfile = profile; }

    void addRegressionForward(const QVector<QPointF>& points);
    void addRegressionBackward(const QVector<QPointF>& points);
    void addFriction(const QVector<QPointF>& points);
    void applyManualMainRegression(quint8 seriesN, QList<QPointF> points);
    void onMainResultReceived(const Domain::Tests::Main::Result& result);

    void setDacReal(qreal value);

    void startStrokeTest();
    void startMainTest(const Domain::Tests::Main::Params& params);
    void startResponseTest(const Domain::Tests::Option::Params& params);
    void startResolutionTest(const Domain::Tests::Option::Params& params);
    void startStepTest(const Domain::Tests::Option::Step::Params& params);
    void startCyclicTest(const Domain::Tests::Cyclic::Params& params);
    void runCombinedCyclicTest(const Domain::Tests::Cyclic::Params& params);

    void endTest();
    void terminateTest();

    void button_set_position();
    void button_DO(quint8 DO_num, bool state);
    void checkbox_autoInit(int state);
};
}