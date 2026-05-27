#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QPointF>
#include <QThread>
#include <QPlainTextEdit>
#include <QElapsedTimer>

#include "Utils/Shortcuts/TabActionRouter.h"

#include "Gui/MainWindow/TelemetryUiMapper.h"
#include "Gui/MainWindow/CrossingIndicatorsPresenter.h"
#include "Gui/TestSettings/MainTestSettings.h"
#include "Gui/TestSettings/CyclicTestSettings.h"
#include "Gui/TestSettings/OtherTestSettings.h"
#include "Gui/TestSettings/StepTestSettings.h"
#include "Gui/TestSettings/BaseSequenceSettingsDialog.h"

#include "Report/Saver.h"

#include "Domain/Program.h"
#include "Storage/Registry.h"
#include "Storage/Telemetry.h"
#include "Storage/ChartImageStorage.h"

#include "Widgets/Chart/Manager.h"

namespace Widgets::Chart {
class ImageService;
}

#include "TestController.h"

// QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
// QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setRegistry(Registry *registry);
    void setProfile(const Domain::DeviceProfile& profile) {
        m_deviceProfile = profile;
        updateAvailableTabs();
    }
signals:
    void initialized();
    void profileChanged(const Domain::DeviceProfile& profile);

    void doInitStatesSelected(const QVector<bool> &states);
    void dacValueRequested(qreal value);

    void stopTest();

    void setDo(quint8 doIndex, bool state);

private slots:
    void setSensorsMask(quint8 mask);
    void appendLog(const QString& text);

    void onTelemetryUpdated(const Telemetry &telemetry);


    void promptSaveChartsAfterTest();

    void directoryToSave(const QString &currentPath, QString &result);

    void setTask(qreal task);

    void onMainResultUpdated(const Domain::Tests::Main::Result& result);
    void onStrokeResultUpdated(const Domain::Tests::Stroke::Result& result);
    void onStepResultUpdated(const Domain::Tests::Option::Step::Result& result);
    void onCyclicRegulatoryResultUpdated(const Domain::Tests::Cyclic::Regulatory::Result& result);
    void onCyclicShutoffResultUpdated(const Domain::Tests::Cyclic::Shutoff::Result& result);
    void onCrossingStatusUpdated(const CrossingStatus& status);
    void setSensorsNumber(quint8 sensorCount);

    void setRegressionEnabled(bool enabled);

    void setDoButtonsChecked(quint8 bitmask);
    void setDiCheckboxesChecked(quint8 bitmask);
    void onSampleReady(const Domain::Measurement::Sample& s);

    void setTaskControlsEnabled(bool enabled);

    void onCountdownTimeout();
    void onTotalTestTimeMs(quint64 totalMs);

    bool askQuestion(const QString &title, const QString &text);

    void endTest();

    void onCyclicTestParametersRequested(Domain::Tests::Cyclic::Params& parameters);

    void setupUiConnections();
    void startMainTestClicked();
    void saveMainTestChartClicked();
    void startStrokeTestClicked();
    void saveStrokeChartClicked();
    void startOptionalTestClicked();
    void saveOptionalTestChartClicked();
    void startCyclicTestClicked();
    void saveCyclicChartClicked();
    void initClicked();
    void generateReportClicked();
    void openReportClicked();
    void backClicked();
    void setupArrowButton(QToolButton* button,
                          const QString& normalIcon,
                          const QString& hoverIcon,
                          double step);

private:
    Ui::MainWindow *ui;

    TabActionRouter m_tabActionRouter;
    std::unique_ptr<TelemetryUiMapper> m_mapper;
    std::unique_ptr<CrossingIndicatorsPresenter> m_crossingIndicators;
    std::unique_ptr<Widgets::Chart::Manager> m_chartManager;
    Widgets::Chart::ImageService* m_chartImages;

    Registry *m_registry = nullptr;
    Telemetry m_telemetry;

    Domain::Program *m_program;
    QThread *m_programThread;
    TestController* m_testController = nullptr;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;
    quint64 m_totalTestMs;

    Report::ReportData m_report;
    Report::Saver* m_reportSaver = nullptr;

    QPlainTextEdit* m_logOutput = nullptr;

    bool m_isInitialized = false;
    bool m_chartsInitialized = false;

    void saveChart(Widgets::Chart::ChartType chart);
    QLabel* previewLabelForChart(Widgets::Chart::ChartType chart) const;

    bool tryStartTest();

    void setupShortcuts();
    void setupPrimaryActions();

    void updateCyclicLabels(const Domain::Tests::Cyclic::Params& parameters);
    qint64 cyclicTotalTimeMs(const Domain::Tests::Cyclic::Params& parameters) const;

    TestState m_testState = TestState::Idle;
    void applyTestStateToUi(TestState  state);
    void setTestState(TestState state);
    void showIdleState();
    void collectRegistryOverrides(ObjectInfo& objectInfo, ValveInfo& valveInfo, OtherParameters& otherParameters);

    void collectReportOverrides();

    void lockTabsForPreInit();
    void updateAvailableTabs();
    void applyCrossingLimitsFromRecommend(const ValveInfo& valveInfo);
    QVector<Widgets::Chart::ChartType> chartsForCurrentTest() const;

    struct SeriesVisibilityBackup {
        QVector<bool> visible;
    };

    void restoreSeries(Widgets::Chart::ChartType chart, const SeriesVisibilityBackup& b);

    Domain::DeviceProfile m_deviceProfile;

    QVector<BaseSequenceSettingsDialog*> m_testSettings;
    MainTestSettings *m_mainTestSettings;
    StepTestSettings *m_stepTestSettings;
    OtherTestSettings *m_responseTestSettings;
    OtherTestSettings *m_resolutionTestSettings;
    CyclicTestSettings *m_cyclicTestSettings;

    ChartImageStorage m_chartImageStorage;

    QHash<QString, QString> m_testComments;

    void editTestComment(const QString& testKey, const QString& description);

    bool hasAnyTestComments() const;
    void saveCommentsPdfIfNeeded();
    QString buildCommentsHtml() const;
    QString commentsPdfPath() const;

    void syncTaskChartSeriesVisibility(quint8 sensorCount);
    void displayDependingPattern();

    void applyProfileVisibility();
    void initCharts();

    void getImage(QLabel* label, Widgets::Chart::ChartType chart);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
