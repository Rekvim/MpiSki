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
    void SetSensorsNumber(quint8 num);
    // void SetSensorsNumber(quint8 num);

private slots:

    void AddPoints(Charts chart, QVector<Point> points);
    void ClearPoints(Charts chart);

    void promptSaveCharts();
    void SetChartVisible(Charts chart, quint16 series, bool visible);
    void ShowDots(bool visible);
    void DublSeries();
    void EnableSetTask(bool enable);

    void Question(QString title, QString text, bool &result);

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
    void SetTextColor(const TextObjects object, const QColor color);
    void SetStepTestResults(QVector<StepTest::TestResult> results, quint32 T_value);
    void SetSolenoidResults(QString sequence, quint16 cycles, double totalTimeSec);
    void SetButtonInitEnabled(bool enable);
    void SetRegressionEnable(bool enable);

    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

    void ButtonStartMain();
    void ButtonStartStroke();
    void ButtonStartOptional();
    void ButtonStartCyclicSolenoid();

    void onCountdownTimeout();

private:
    Ui::MainWindow *ui;

    QTimer      m_cyclicCountdownTimer;
    QElapsedTimer m_cyclicElapsedTimer;
    qint64      m_cyclicTotalMs = 0;

    Registry *m_registry = nullptr;
    TestTelemetryData collectTestTelemetryData() const;
    QString m_lastSolenoidSequence;

    ReportSaver *m_reportSaver = nullptr;
    Program *m_program;
    QThread *m_programthread;

    QTimer* m_durationTimer;
    QElapsedTimer m_elapsedTimer;
    qint64 m_totalTestMs;

    ReportSaver::Report m_report;

    SelectTests::PatternType m_patternType = SelectTests::Pattern_None;
    SelectTests::BlockCTS m_blockCTS;

    QHash<TextObjects, QLabel *> m_labels;
    QHash<TextObjects, QLineEdit *> m_lineEdits;
    QHash<Charts, MyChart *> m_charts;

    MainTestSettings *m_mainTestSettings;
    StepTestSettings *m_stepTestSettings;
    OtherTestSettings *m_responseTestSettings;
    OtherTestSettings *m_resolutionTestSettings;
    CyclicTestSettings *m_cyclicTestSettings;

    QImage m_image_1;
    QImage m_image_2;
    QImage m_image_3;

    void onCyclicCountdown();
    bool m_testing;
    void InitCharts();
    void SaveChart(Charts chart);
    void GetImage(QLabel *label, QImage *image);
    void InitReport();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void StartCyclicSolenoidTest(const CyclicTestSettings::TestParameters &p);
    void SetDAC(qreal value);
    void StartMainTest();
    void StartStrokeTest();
    void StartOptionalTest(quint8 testNum);
    void StopTest();
    void SetDO(quint8 DO_num, bool state);
};
#endif // MAINWINDOW_H
