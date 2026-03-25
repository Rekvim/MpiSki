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

#include "./Src/ReportBuilders/ReportSaver.h"
#include "Program.h"
#include "./Src/Storage/Registry.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include "./Src/Ui/TestSettings/AbstractTestSettings.h"
#include "./Src/CustomChart/ChartManager.h"

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

    // void runMainTest();
    // void runStrokeTest();
    // void runCyclicTest();
    // void runOptionalTest(quint8 testNum);

    void stopTest();

    void setDo(quint8 doIndex, bool state);

private slots:
    void setSensorsMask(quint8 mask);
    void appendLog(const QString& text);

    void onTelemetryUpdated(const TelemetryStore &telemetry);


    void promptSaveChartsAfterTest();

    void getDirectory(const QString &currentPath, QString &result);

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

    void askQuestion(const QString &title, const QString &text, bool &result);

    void endTest();

    void onStrokeTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);
    void onMainTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);
    void onStepTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);
    void onCyclicTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart);

    // void onMainTestParametersRequested(MainTestSettings::TestParameters &parameters);
    // void onStepTestParametersRequested(StepTestSettings::TestParameters &parameters);
    // void onResolutionTestParametersRequested(OtherTestSettings::TestParameters &parameters);
    // void onResponseTestParametersRequested(OtherTestSettings::TestParameters &parameters);
    void onCyclicTestParametersRequested(CyclicTestSettings::TestParameters &parameters);

    void on_pushButton_init_clicked();

    void on_pushButton_report_generate_clicked();
    void on_pushButton_report_open_clicked();

    void on_pushButton_mainTest_start_clicked();
    void on_pushButton_mainTest_save_clicked();

    void on_pushButton_strokeTest_start_clicked();
    void on_pushButton_strokeTest_save_clicked();

    void on_pushButton_optionalTests_start_clicked();
    void on_pushButton_optionalTests_save_clicked();

    void on_pushButton_cyclicTest_start_clicked();
    void on_pushButton_cyclicTest_save_clicked();

    void on_pushButton_imageChartTask_clicked();
    void on_pushButton_imageChartPressure_clicked();
    void on_pushButton_imageChartFriction_clicked();

    void on_pushButton_signal_4mA_clicked();
    void on_pushButton_signal_8mA_clicked();
    void on_pushButton_signal_12mA_clicked();
    void on_pushButton_signal_16mA_clicked();
    void on_pushButton_signal_20mA_clicked();

    void on_pushButton_back_clicked();

private:
    Ui::MainWindow *ui;

    TabActionRouter m_tabActionRouter;
    std::unique_ptr<TelemetryUiMapper> m_mapper;
    std::unique_ptr<CrossingIndicatorsPresenter> m_crossingIndicators;
    std::unique_ptr<ChartManager> m_chartManager;

    Registry *m_registry = nullptr;
    TelemetryStore m_telemetryStore;

    Program *m_program;
    QThread *m_programThread;
    TestController* m_testController = nullptr;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;
    quint64 m_totalTestMs;

    ReportSaver::Report m_report;
    ReportSaver *m_reportSaver = nullptr;

    QPlainTextEdit* m_logOutput = nullptr;

    bool m_isInitialized = false;
    bool m_chartsInitialized = false;

    bool tryStartTest();

    void setupShortcuts();
    void setupPrimaryActions();

    void setTestState(TestState state);
    TestState m_testState = TestState::Idle;

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

    SeriesVisibilityBackup hideTaskAuxSeries();
    SeriesVisibilityBackup hidePressureAuxSeries();
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
    void saveChart(Charts chart);
    void getImage(QLabel* label, QImage* image);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
