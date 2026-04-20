#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointF>
#include <QThread>
#include <QDebug>
#include <QPlainTextEdit>
#include <QElapsedTimer>

#include "Src/Utils/Shortcuts/TabActionRouter.h"

#include "Src/Ui/MainWindow/TelemetryUiMapper.h"
#include "Src/Ui/MainWindow/CrossingIndicatorsPresenter.h"

#include "Src/Ui/TestSettings/MainTestSettings.h"
#include "Src/Ui/TestSettings/CyclicTestSettings.h"
#include "Src/Ui/TestSettings/OtherTestSettings.h"
#include "Src/Ui/TestSettings/StepTestSettings.h"

#include "Src/Report/Saver.h"
#include "Src/Domain/Program.h"
#include "Src/Storage/Registry.h"
#include "Src/Storage/Telemetry.h"
#include "Src/Ui/TestSettings/AbstractTestSettings.h"
#include "Src/CustomChart/ChartManager.h"
#include "Src/CustomChart/ChartImageService.h"

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
    void setPatternType(SelectTests::PatternType pattern) {
        m_patternType = pattern;
        updateAvailableTabs();
    }
signals:
    void initialized();
    void patternChanged(SelectTests::PatternType pattern);

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

    void setText(TextObjects object, const QString &text);
    void setTask(qreal task);

    void setStepTestResults(const QVector<StepTest::TestResult> &results, quint32 T_value);

    void setSensorsNumber(quint8 sensorCount);

    void setButtonInitEnabled(bool enabled);
    void setRegressionEnabled(bool enabled);

    void setDoButtonsChecked(quint8 bitmask);
    void setDiCheckboxesChecked(quint8 bitmask);

    void setTaskControlsEnabled(bool enabled);

    void onCountdownTimeout();
    void onTotalTestTimeMs(quint64 totalMs);

    bool askQuestion(const QString &title, const QString &text);

    void endTest();

    void onStrokeTestPointsRequested(QVector<QVector<QPointF>>& points, Charts chart);
    void onMainTestPointsRequested(QVector<QVector<QPointF>>& points, Charts chart);
    void onStepTestPointsRequested(QVector<QVector<QPointF>>& points, Charts chart);
    void onCyclicTestPointsRequested(QVector<QVector<QPointF>>& points, Charts chart);

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
    std::unique_ptr<ChartManager> m_chartManager;
    ChartImageService* m_chartImages;

    Registry *m_registry = nullptr;
    Telemetry m_telemetry;

    Program *m_program;
    QThread *m_programThread;
    TestController* m_testController = nullptr;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;
    quint64 m_totalTestMs;

    Report::Saver::Report m_report;
    Report::Saver *m_reportSaver = nullptr;

    QPlainTextEdit* m_logOutput = nullptr;

    bool m_isInitialized = false;
    bool m_chartsInitialized = false;

    void saveChart(Charts chart);
    bool tryStartTest();

    void setupShortcuts();
    void setupPrimaryActions();


    TestState m_testState = TestState::Idle;
    void applyTestStateToUi(TestState  state);
    void setTestState(TestState state);
    void showInitializingState();
    void showIdleState();
    void collectRegistryOverrides(
        ObjectInfo& objectInfo,
        ValveInfo& valveInfo,
        OtherParameters& otherParameters
    );

    void collectReportOverrides();

    void lockTabsForPreInit();
    void updateAvailableTabs();
    void applyCrossingLimitsFromRecommend(const ValveInfo& valveInfo);
    QVector<Charts> chartsForCurrentTest() const;

    struct SeriesVisibilityBackup {
        QVector<bool> visible;
    };

    void restoreSeries(Charts chart, const SeriesVisibilityBackup& b);

    SelectTests::PatternType m_patternType = SelectTests::Pattern_None;

    QHash<TextObjects, QLineEdit *> m_lineEdits;

    QVector<AbstractTestSettings*> m_testSettings;
    MainTestSettings *m_mainTestSettings;
    StepTestSettings *m_stepTestSettings;
    OtherTestSettings *m_responseTestSettings;
    OtherTestSettings *m_resolutionTestSettings;
    CyclicTestSettings *m_cyclicTestSettings;

    QImage m_imageChartTask;
    QImage m_imageChartPressure;
    QImage m_imageChartFriction;
    QImage m_imageChartStep;

    void syncTaskChartSeriesVisibility(quint8 sensorCount);
    void displayDependingPattern();

    void initCharts();

    void getImage(QLabel* label, QImage* image);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
