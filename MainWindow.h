#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointF>
#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <QPlainTextEdit>

#include "./Src/ReportBuilders/ReportSaver.h"
#include "Program.h"
#include "Registry.h"
#include "SelectTests.h"
#include "./Src/Telemetry/TelemetryStore.h"

// QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
// QT_END_NAMESPACE

struct CTSRule {
    std::function<bool(const SelectTests::BlockCTS&)> condition;
    std::array<bool, 6> pattern;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void SetRegistry(Registry *registry);
    void SetPatternType(SelectTests::PatternType pattern) { m_patternType = pattern; }
    void SetBlockCTS(const SelectTests::BlockCTS& cts) { m_blockCTS = cts; }
signals:
    void initialize();
    void PatternChanged(SelectTests::PatternType pattern);

    void InitDOSelected(const QVector<bool> &states);
    void SetDAC(qreal value);

    void runMainTest();
    void runStrokeTest();
    void runCyclicTest();
    void runOptionalTest(quint8 testNum);

    void stopTheTest();

    void SetDO(quint8 DO_num, bool state);

private slots:
    void appendLog(const QString& text);

    void onTelemetryUpdated(const TelemetryStore &TS);
    void SetSensorsNumber(quint8 num);
    void AddPoints(Charts chart, const QVector<Point> &points);
    void ClearPoints(Charts chart);

    void promptSaveCharts();
    void SetChartVisible(Charts chart, quint16 series, bool visible);
    void ShowDots(bool visible);
    void DublSeries();

    void startTest();
    void endTest();

    void GetDirectory(const QString &current_path, QString &result);

    void receivedPoints_mainTest(QVector<QVector<QPointF>> &points, Charts chart);
    void receivedPoints_optionTest(QVector<QVector<QPointF>> &points, Charts chart);
    void receivedPoints_cyclicTest(QVector<QVector<QPointF>> &points, Charts chart);

    void receivedParameters_mainTest(MainTestSettings::TestParameters &parameters);
    void receivedParameters_stepTest(StepTestSettings::TestParameters &parameters);
    void receivedParameters_resolutionTest(OtherTestSettings::TestParameters &parameters);
    void receivedParameters_responseTest(OtherTestSettings::TestParameters &parameters);
    void receivedParameters_cyclicTest(CyclicTestSettings::TestParameters &parameters);

    void updateFrictionForceLimitStatus();
    void updateRangeLimitStatus();
    void updateDynamicErrorLimitStatus();

    void bindSliderAndLineEdit(QSlider* slider, QLineEdit* lineEdit, std::function<void()> updateIndicatorFn);


    void SetText(TextObjects object, const QString &text);

    void SetTask(qreal task);
    void EnableSetTask(bool enable);

    void SetButtonInitEnabled(bool enable);
    void SetRegressionEnable(bool enable);

    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

    void onCountdownTimeout();
    void onTotalTestTimeMs(quint64 totalMs);

    void Question(const QString &title, const QString &text, bool &result);

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

private:
    Ui::MainWindow *ui;
    TelemetryStore m_telemetryStore;

    QPlainTextEdit* logOutput = nullptr;

    bool m_userCanceled = false;
    bool m_testing = false;

    Registry *m_registry = nullptr;

    ReportSaver *m_reportSaver = nullptr;
    Program *m_program;
    QThread *m_programThread;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;

    quint64 m_totalTestMs;

    ReportSaver::Report m_report;

    SelectTests::PatternType m_patternType = SelectTests::Pattern_None;
    SelectTests::BlockCTS m_blockCTS;

    QHash<TextObjects, QLineEdit *> m_lineEdits;
    QHash<Charts, MyChart *> m_charts;

    MainTestSettings *m_mainTestSettings;
    StepTestSettings *m_stepTestSettings;
    OtherTestSettings *m_responseTestSettings;
    OtherTestSettings *m_resolutionTestSettings;
    CyclicTestSettings *m_cyclicTestSettings;

    QImage m_imageChartTask;
    QImage m_imageChartPressure;
    QImage m_imageChartFriction;
    QImage m_imageChartStep;

    void DisplayDependingPattern();

    void InitCharts();
    void SaveChart(Charts chart);
    void GetImage(QLabel *label, QImage *image);

    void SetStepTestResults(const QVector<StepTest::TestResult> &results, quint32 T_value);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
