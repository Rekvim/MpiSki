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

#include "./Src/ReportBuilders/ReportSaver.h"
#include "Program.h"
#include "Registry.h"
#include "SelectTests.h"
#include "./Src/Telemetry/TelemetryStore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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

private slots:
    void onTelemetryUpdated(const TelemetryStore &TS);
    void SetSensorsNumber(quint8 num);
    void AddPoints(Charts chart, QVector<Point> points);
    void ClearPoints(Charts chart);

    void promptSaveCharts();
    void SetChartVisible(Charts chart, quint16 series, bool visible);
    void ShowDots(bool visible);
    void DublSeries();
    void EnableSetTask(bool enable);


    void StartTest();
    void EndTest();

    void GetDirectory(QString current_path, QString &result);
    void GetPoints(QVector<QVector<QPointF>> &points, Charts chart);

    void GetMainTestParameters(MainTestSettings::TestParameters &parameters);
    void GetStepTestParameters(StepTestSettings::TestParameters &parameters);
    void GetResolutionTestParameters(OtherTestSettings::TestParameters &parameters);
    void GetResponseTestParameters(OtherTestSettings::TestParameters &parameters);
    void GetCyclicTestParameters(CyclicTestSettings::TestParameters &parameters);

    void SetText(const TextObjects object, const QString &text);
    void SetTask(qreal task);
    void SetButtonInitEnabled(bool enable);
    void SetRegressionEnable(bool enable);

    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

    void onCountdownTimeout();

    void Question(QString title, QString text, bool &result);

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

    QTimer m_cyclicCountdownTimer;
    QElapsedTimer m_cyclicElapsedTimer;
    qint64 m_cyclicTotalMs = 0;
    bool m_userCanceled = false;
    bool m_testing;

    Registry *m_registry = nullptr;

    ReportSaver *m_reportSaver = nullptr;
    Program *m_program;
    QThread *m_programThread;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;
    qint64 m_totalTestMs;

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

    void onCyclicCountdown();
    void InitCharts();
    void SaveChart(Charts chart);
    void GetImage(QLabel *label, QImage *image);

    void SetStepTestResults(QVector<StepTest::TestResult> results, quint32 T_value);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void Initialize();
    void PatternChanged(SelectTests::PatternType pattern);

    void InitDOSelected(const QVector<bool> &states);
    void SetDAC(qreal value);

    void runMainTest();
    void runStrokeTest();
    void runCyclicTest(const CyclicTestSettings::TestParameters &p);
    void runOptionalTest(quint8 testNum);

    void stopTheTest();

    void SetDO(quint8 DO_num, bool state);
};
#endif // MAINWINDOW_H
