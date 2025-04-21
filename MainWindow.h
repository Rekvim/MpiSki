#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointF>
#include <QThread>

#include "FileSaver.h"
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
    void SetBlockCTS(const SelectTests::BlockCTS &blockCTS);
    void SetSensorsNumber(quint8 num, bool apply_logic = true);
    // void SetSensorsNumber(quint8 num);

private slots:

    void AddPoints(Charts chart, QVector<Point> points);
    void ClearPoints(Charts chart);

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
    void SetSolenoidResults(qint64 forward, qint64 backward, quint16 cycles, double range_percent, double total_time_sec);
    void SetButtonInitEnabled(bool enable);
    void SetRegressionEnable(bool enable);

    void SetButtonsDOChecked(quint8 status);
    void SetCheckboxDIChecked(quint8 status);

    void ButtonStartMain();
    void ButtonStartStroke();
    void ButtonStartOptional();

private:
    Ui::MainWindow *ui;

    Registry *m_registry = nullptr;
    FileSaver *m_fileSaver = nullptr;
    Program *m_program;
    QThread *m_programthread;

    FileSaver::Report m_report;
    SelectTests::BlockCTS m_blockCTS;

    QHash<TextObjects, QLabel *> m_labels;
    QHash<TextObjects, QLineEdit *> m_lineEdits;
    QHash<Charts, MyChart *> m_charts;

    MainTestSettings *m_mainTestSettings;
    StepTestSettings *m_stepTestSettings;
    OtherTestSettings *m_responseTestSettings;
    OtherTestSettings *m_resolutionTestSettings;
    CyclicTestSettings *m_cyclicTestSettings;

    bool m_testing;

    void InitCharts();
    void SaveChart(Charts chart);
    void GetImage(QLabel *label, QImage *image);
    void InitReport();
signals:
    void SetDAC(qreal value);
    void StartMainTest();
    void StartStrokeTest();
    void StartOptionalTest(quint8 test_num);
    void StopTest();
    void SetDO(quint8 DO_num, bool state);
};
#endif // MAINWINDOW_H
