#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CyclicTestSettings.h"

#include "Src/ReportBuilders/ReportBuilder_B_CVT.h"
#include "Src/ReportBuilders/ReportBuilder_B_SACVT.h"
#include "Src/ReportBuilders/ReportBuilder_C_CVT.h"
#include "Src/ReportBuilders/ReportBuilder_C_SACVT.h"
#include "Src/ReportBuilders/ReportBuilder_C_SOVT.h"
#include "./Src/Tests/CyclicTestSolenoid.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_testing = false;

    ui->tabWidget->setCurrentIndex(0);

    m_durationTimer = new QTimer(this);
    m_durationTimer->setInterval(100);

    connect(m_durationTimer, &QTimer::timeout,
            this, &MainWindow::onCountdownTimeout);

    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tab_mainTests), false);
    ui->tabWidget->setTabEnabled(2, true);
    ui->tabWidget->setTabEnabled(3, true);
    ui->tabWidget->setTabEnabled(4, true);

    m_cyclicCountdownTimer.setInterval(100);
    connect(&m_cyclicCountdownTimer, &QTimer::timeout,
            this, &MainWindow::onCyclicCountdown);

    m_mainTestSettings = new MainTestSettings(this);
    m_stepTestSettings = new StepTestSettings(this);
    m_responseTestSettings = new OtherTestSettings(this);
    m_resolutionTestSettings = new OtherTestSettings(this);
    m_cyclicTestSettings = new CyclicTestSettings(this);

    m_reportSaver = new ReportSaver(this);

    ui->checkBox_switch_3_0->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_switch_3_0->setFocusPolicy(Qt::NoFocus);
    ui->checkBox_switch_0_3->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_switch_0_3->setFocusPolicy(Qt::NoFocus);

    m_labels[TextObjects::Label_deviceStatusValue] = ui->label_deviceStatusValue;
    m_labels[TextObjects::Label_deviceInitValue] = ui->label_deviceInitValue;
    m_labels[TextObjects::Label_connectedSensorsNumber] = ui->label_connectedSensorsNumber;
    m_labels[TextObjects::Label_startingPositionValue] = ui->label_startingPositionValue;
    m_labels[TextObjects::Label_finalPositionValue] = ui->label_finalPositionValue;
    m_labels[TextObjects::Label_lowLimitValue] = ui->label_lowLimitValue;
    m_labels[TextObjects::Label_highLimitValue] = ui->label_highLimitValue;
    m_labels[TextObjects::Label_pressureDifferenceValue] = ui->label_pressureDifferenceValue;
    m_labels[TextObjects::Label_frictionForceValue] = ui->label_frictionForceValue;
    m_labels[TextObjects::label_frictionPercentValue] = ui->label_frictionPercentValue;
    m_labels[TextObjects::Label_dynamicErrorMean] = ui->label_dynamicErrorMean;
    m_labels[TextObjects::Label_dynamicErrorMeanPercent] = ui->label_dynamicErrorMeanPercent;
    m_labels[TextObjects::Label_dynamicErrorMax] = ui->label_dynamicErrorMax;
    m_labels[TextObjects::Label_dynamicErrorMaxPercent] = ui->label_dynamicErrorMaxPercent;
    m_labels[TextObjects::Label_valveStroke_range] = ui->label_valveStroke_range;
    m_labels[TextObjects::Label_strokeTest_forwardTime] = ui->label_strokeTest_forwardTime;
    m_labels[TextObjects::Label_strokeTest_backwardTime] = ui->label_strokeTest_backwardTime;

    m_lineEdits[TextObjects::LineEdit_linearSensor] = ui->lineEdit_linearSensor;
    m_lineEdits[TextObjects::LineEdit_linearSensorPercent] = ui->lineEdit_linearSensorPercent;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_1] = ui->lineEdit_pressureSensor_1;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_2] = ui->LineEdit_pressureSensor_2;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_3] = ui->LineEdit_pressureSensor_3;
    m_lineEdits[TextObjects::LineEdit_feedback_4_20mA] = ui->lineEdit_feedback_4_20mA;
    m_lineEdits[TextObjects::LineEdit_dinamicReal] = ui->lineEdit_dynamicErrorReal;
    m_lineEdits[TextObjects::lineEdit_strokeReal] = ui->lineEdit_strokeReal;
    m_lineEdits[TextObjects::lineEdit_rangeReal] = ui->lineEdit_driveRangeReal;
    m_lineEdits[TextObjects::LineEdit_friction] = ui->lineEdit_friction;
    m_lineEdits[TextObjects::LineEdit_frictionPercent] = ui->lineEdit_frictionPercent;
    m_lineEdits[TextObjects::LineEdit_strokeTest_forwardTime] = ui->lineEdit_strokeTest_forwardTime;
    m_lineEdits[TextObjects::LineEdit_strokeTest_backwardTime] = ui->lineEdit_strokeTest_backwardTime;
    m_lineEdits[TextObjects::LineEdit_rangePressure] = ui->lineEdit_rangePressure;

    m_program = new Program;
    m_programThread = new QThread(this);
    m_program->moveToThread(m_programThread);

    connect(ui->pushButton_init, &QPushButton::clicked,
            m_program, &Program::button_init);

    connect(ui->pushButton_set, &QPushButton::clicked,
            m_program, &Program::button_set_position);


    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            m_program, &Program::checkbox_autoinit);

    connect(this, &MainWindow::SetDO,
            m_program, &Program::button_DO);

    for (int i = 0; i < 4; ++i) {
        auto btn = findChild<QPushButton*>(QString("pushButton_DO%1").arg(i));
        if (!btn) continue;

        connect(btn, &QPushButton::clicked,
                this, [this, i](bool checked)
                {
                    emit SetDO(i, checked);
                });
    }

    connect(ui->pushButton_mainTest_start, &QPushButton::clicked,
            this, &MainWindow::ButtonStartMain);

    connect(ui->pushButton_mainTest_save, &QPushButton::clicked, this, [&] {
        if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_task) {
            SaveChart(Charts::Task);
        } else if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_pressure) {
            SaveChart(Charts::Pressure);
        } else if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_friction) {
            SaveChart(Charts::Friction);
        }
    });

    connect(ui->pushButton_strokeTest_start,
            &QPushButton::clicked,
            this,
            &MainWindow::ButtonStartStroke);

    connect(ui->pushButton_strokeTest_save, &QPushButton::clicked, this, [&] {
        SaveChart(Charts::Stroke);
    });

    connect(ui->pushButton_optionalTests_start, &QPushButton::clicked,
            this, &MainWindow::ButtonStartOptional);

    connect(this, &MainWindow::StartCyclicSolenoidTest,
            m_program, &Program::CyclicSolenoidTestStart);

    connect(ui->pushButton_optionalTests_save, &QPushButton::clicked, this, [&] {
        if (ui->tabWidget_tests->currentWidget() == ui->tab_optionalTests_response) {
            SaveChart(Charts::Response);
        } else if (ui->tabWidget_tests->currentWidget() == ui->tab_optionalTests_resolution) {
            SaveChart(Charts::Resolution);
        } else if (ui->tabWidget_tests->currentWidget() == ui->tab_optionalTests_step) {
            SaveChart(Charts::Step);
        }
    });

    connect(ui->pushButton_cyclicTest_start, &QPushButton::clicked,
            this, &MainWindow::ButtonStartCyclicSolenoid);

    connect(ui->pushButton_cyclicTest_save, &QPushButton::clicked,
            this, [&](){
                SaveChart(Charts::CyclicSolenoid);
            });


    connect(ui->pushButton_open, &QPushButton::clicked,
            m_program, &Program::button_open);

    connect(ui->pushButton_report, &QPushButton::clicked,
            m_program, &Program::button_report);

    connect(ui->pushButton_imageChartTask, &QPushButton::clicked,
            m_program, &Program::button_pixmap1);

    connect(ui->pushButton_imageChartPressure, &QPushButton::clicked,
            m_program, &Program::button_pixmap2);

    connect(ui->pushButton_imageChartFriction, &QPushButton::clicked,
            m_program, &Program::button_pixmap3);

    connect(this, &MainWindow::StartMainTest,
            m_program, &Program::MainTestStart);

    connect(m_program, &Program::MainTestFinished,
            this, &MainWindow::promptSaveCharts);

    connect(this, &MainWindow::StartStrokeTest,
            m_program, &Program::StrokeTestStart);

    connect(this, &MainWindow::StartOptionalTest,
            m_program, &Program::StartOptionalTest);

    connect(this, &MainWindow::StopTest,
            m_program, &Program::TerminateTest);

    connect(m_program, &Program::StopTest,
            this, &MainWindow::EndTest);

    connect(m_program, &Program::SetText,
            this, &MainWindow::SetText);

    connect(m_program, &Program::SetTextColor,
            this, &MainWindow::SetTextColor);

    connect(m_program, &Program::SetSolenoidResults,
            this, &MainWindow::SetSolenoidResults);

    connect(m_program, &Program::SetGroupDOVisible,
            this, [&](bool visible) {
                ui->groupBox_DO->setVisible(visible);
            });

    connect(m_program, &Program::SetButtonsDOChecked,
            this, &MainWindow::SetButtonsDOChecked);

    connect(m_program, &Program::SetCheckboxDIChecked,
            this, &MainWindow::SetCheckboxDIChecked);

    connect(this, &MainWindow::SetDAC,
            m_program, &Program::SetDAC_real);

    connect(ui->doubleSpinBox_task,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,[&](double value) {
                if (qRound(value * 1000) != ui->verticalSlider_task->value()) {
                    if (ui->verticalSlider_task->isEnabled())
                        emit SetDAC(value);
                    ui->verticalSlider_task->setValue(qRound(value * 1000));
                }
            });

    connect(ui->verticalSlider_task, &QSlider::valueChanged,
            this, [&](int value) {
                if (qRound(ui->doubleSpinBox_task->value() * 1000) != value) {
                    if (ui->doubleSpinBox_task->isEnabled())
                        emit SetDAC(value / 1000.0);
                    ui->doubleSpinBox_task->setValue(value / 1000.0);
                }
            });

    connect(ui->pushButton_signal_4mA, &QPushButton::clicked,
            this, [this]() {
                ui->doubleSpinBox_task->setValue(4.0);
            });

    connect(ui->pushButton_signal_8mA, &QPushButton::clicked,
            this, [this]() {
                ui->doubleSpinBox_task->setValue(8.0);
            });

    connect(ui->pushButton_signal_12mA, &QPushButton::clicked,
            this, [this]() {
                ui->doubleSpinBox_task->setValue(12.0);
            });

    connect(ui->pushButton_signal_16mA, &QPushButton::clicked, this, [this]() {
        ui->doubleSpinBox_task->setValue(16.0);
    });

    connect(ui->pushButton_signal_20mA, &QPushButton::clicked, this, [this]() {
        ui->doubleSpinBox_task->setValue(20.0);
    });

    connect(m_program, &Program::SetTask,
            this, &MainWindow::SetTask);

    connect(m_program, &Program::SetSensorNumber,
            this, &MainWindow::SetSensorsNumber);

    connect(m_program, &Program::SetButtonInitEnabled,
            this, &MainWindow::SetButtonInitEnabled);

    connect(m_program, &Program::EnableSetTask,
            this, &MainWindow::EnableSetTask);

    connect(m_program, &Program::SetStepResults,
            this, &MainWindow::SetStepTestResults);

    connect(m_program, &Program::GetMainTestParameters,
            this, &MainWindow::GetMainTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::GetStepTestParameters,
            this, &MainWindow::GetStepTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::GetResolutionTestParameters,
            this, &MainWindow::GetResolutionTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::GetResponseTestParameters,
            this, &MainWindow::GetResponseTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::GetCyclicTestParameters,
            this, &MainWindow::GetCyclicTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::Question,
            this, &MainWindow::Question,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::SetSolenoidRangesData,
            this, &MainWindow::onSolenoidRangesData);

    connect(m_reportSaver, &ReportSaver::Question,
            this, &MainWindow::Question,
            Qt::DirectConnection);

    connect(m_reportSaver, &ReportSaver::GetDirectory,
            this, &MainWindow::GetDirectory,
            Qt::DirectConnection);

    connect(ui->pushButton_imageChartTask,
            &QPushButton::clicked, this, [&] {
                GetImage(ui->label_imageChartTask, &m_imageChartTask);
            });
    connect(ui->pushButton_imageChartPressure, &QPushButton::clicked,
            this, [&] {
                GetImage(ui->label_imageChartPressure, &m_imageChartPressure);
            });

    connect(ui->pushButton_imageChartFriction, &QPushButton::clicked,
            this, [&] {
                GetImage(ui->label_imageChartFriction, &m_imageChartFriction);
            });

    connect(ui->pushButton_report, &QPushButton::clicked,
            this, [&] {
                collectTestTelemetryData();
                std::unique_ptr<ReportBuilder> reportBuilder;

                switch (m_patternType) {
                case SelectTests::Pattern_B_CVT: reportBuilder = std::make_unique<ReportBuilder_B_CVT>(); break;
                case SelectTests::Pattern_B_SACVT: reportBuilder = std::make_unique<ReportBuilder_B_SACVT>(); break;
                case SelectTests::Pattern_C_CVT: reportBuilder = std::make_unique<ReportBuilder_C_CVT>(); break;
                case SelectTests::Pattern_C_SACVT: reportBuilder = std::make_unique<ReportBuilder_C_SACVT>(); break;
                case SelectTests::Pattern_C_SOVT: reportBuilder = std::make_unique<ReportBuilder_C_SOVT>(); break;
                default:
                    QMessageBox::warning(this, "Ошибка", "Не выбран корректный паттерн отчёта!");
                    return;
                }

                ReportSaver::Report report;
                reportBuilder->buildReport(report,
                                           m_telemetryStore,
                                           *m_registry->GetObjectInfo(),
                                           *m_registry->GetValveInfo(),
                                           *m_registry->GetOtherParameters(),
                                           m_imageChartTask, m_imageChartPressure, m_imageChartFriction, m_imageChartStep);

                qDebug() << "Путь к шаблону:" << reportBuilder->templatePath();

                bool saved = m_reportSaver->SaveReport(report, reportBuilder->templatePath());
                ui->pushButton_open->setEnabled(saved);
            });

    connect(ui->pushButton_open, &QPushButton::clicked,
            this, [&] {
                QDesktopServices::openUrl(
                    QUrl::fromLocalFile(m_reportSaver->Directory().filePath("report.xlsx")));
            });

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                ui->pushButton_set->setEnabled(!state);
            });

    ui->tableWidget_stepResults->setColumnCount(2);
    ui->tableWidget_stepResults->setHorizontalHeaderLabels({"T86", "Перерегулирование"});
    ui->tableWidget_stepResults->resizeColumnsToContents();

    ui->label_arrowUp->setCursor(Qt::PointingHandCursor);
    ui->label_arrowDown->setCursor(Qt::PointingHandCursor);

    ui->label_arrowUp->installEventFilter(this);
    ui->label_arrowDown->installEventFilter(this);

    connect(m_program, &Program::TelemetryUpdated,
            this, [&](const TelemetryStore &store){
                m_telemetryStore = store;
            });
}

MainWindow::~MainWindow()
{
    delete ui;
    m_programThread->quit();
    m_programThread->wait();
    delete m_program;
}

void MainWindow::onCyclicCountdown()
{
    qint64 elapsed = m_cyclicElapsedTimer.elapsed();
    qint64 remaining = m_cyclicTotalMs - elapsed;
    if (remaining < 0) remaining = 0;

    QTime t(0, 0);
    t = t.addMSecs(remaining);
    ui->lineEdit_cyclicTest_totalTime->setText(
        t.toString("hh:mm:ss.zzz"));

    if (remaining == 0) {
        m_cyclicCountdownTimer.stop();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (auto w = qobject_cast<QWidget*>(watched)) {
        if (!w->isEnabled()) {
            return false;
        }
    }
    if (watched == ui->label_arrowUp && event->type() == QEvent::MouseButtonRelease) {
        double cur = ui->doubleSpinBox_task->value();
        double nxt = cur + 0.05;
        if (nxt > ui->doubleSpinBox_task->maximum())
            nxt = ui->doubleSpinBox_task->maximum();
        ui->doubleSpinBox_task->setValue(nxt);
        return true;
    }
    if (watched == ui->label_arrowDown && event->type() == QEvent::MouseButtonRelease) {
        double cur = ui->doubleSpinBox_task->value();
        double nxt = cur - 0.05;
        if (nxt < ui->doubleSpinBox_task->minimum())
            nxt = ui->doubleSpinBox_task->minimum();
        ui->doubleSpinBox_task->setValue(nxt);
        return true;
    }
    if (watched == ui->label_arrowUp) {
        if (event->type() == QEvent::Enter) {
            ui->label_arrowUp->setPixmap(QPixmap(":/Src/Img/arrowUpHover.png"));
            return true;
        }
        if (event->type() == QEvent::Leave) {
            ui->label_arrowUp->setPixmap(QPixmap(":/Src/Img/arrowUp.png"));
            return true;
        }
    }
    if (watched == ui->label_arrowDown) {
        if (event->type() == QEvent::Enter) {
            ui->label_arrowDown->setPixmap(QPixmap(":/Src/Img/arrowDownHover.png"));
            return true;
        }
        if (event->type() == QEvent::Leave) {
            ui->label_arrowDown->setPixmap(QPixmap(":/Src/Img/arrowDown.png"));
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onCountdownTimeout()
{
    qint64 elapsedMs = m_elapsedTimer.elapsed();

    qint64 remainingMs = m_totalTestMs - elapsedMs;
    if (remainingMs <= 0) {
        remainingMs = 0;
    }

    QTime t(0, 0);
    t = t.addMSecs(remainingMs);

    ui->lineEdit_testDuration->setText(t.toString("hh:mm:ss.zzz"));

    if (remainingMs == 0) {
        m_durationTimer->stop();
    }
}

void MainWindow::SetRegistry(Registry *registry)
{
    m_registry = registry;

    ObjectInfo *objectInfo = m_registry->GetObjectInfo();
    ValveInfo *valveInfo = m_registry->GetValveInfo();
    OtherParameters *otherParameters = m_registry->GetOtherParameters();

    ui->lineEdit_date->setText(otherParameters->date);

    ui->lineEdit_object->setText(objectInfo->object);
    ui->lineEdit_manufacture->setText(objectInfo->manufactory);
    ui->lineEdit_department->setText(objectInfo->department);
    ui->lineEdit_FIO->setText(objectInfo->FIO);

    ui->lineEdit_positionNumber->setText(valveInfo->positionNumber);
    ui->lineEdit_manufacturer->setText(valveInfo->manufacturer);
    ui->lineEdit_valveModel->setText(valveInfo->valveModel);
    ui->lineEdit_serialNumber->setText(valveInfo->serialNumber);
    ui->lineEdit_DNPN->setText(valveInfo->DN + "/" + valveInfo->PN);
    ui->lineEdit_driveModel->setText(valveInfo->driveModel);
    ui->lineEdit_positionerModel->setText(valveInfo->positionerModel);
    ui->lineEdit_strokeMovement->setText(otherParameters->strokeMovement);
    ui->lineEdit_safePosition->setText(otherParameters->safePosition);
    ui->lineEdit_dynamicErrorRecomend->setText(QString::number(valveInfo->dinamicErrorRecomend, 'f', 2));
    ui->lineEdit_materialStuffingBoxSeal->setText(valveInfo->materialStuffingBoxSeal);


    ui->lineEdit_strokeRecomend->setText(valveInfo->strokValve);
    ui->lineEdit_driveRangeRecomend->setText(valveInfo->driveRecomendRange);

    if (valveInfo->safePosition != 0) {
        m_stepTestSettings->reverse();
        m_responseTestSettings->reverse();
        m_resolutionTestSettings->reverse();
    }



    DisplayDependingPattern();

    InitCharts();

    m_program->SetRegistry(registry);
    m_programThread->start();

    m_reportSaver->SetRegistry(registry);
}

void MainWindow::SetText(TextObjects object, const QString &text)
{
    if (m_lineEdits.contains(object)) {
        m_lineEdits[object]->setText(text);
    }

    if (m_labels.contains(object)) {
        m_labels[object]->setText(text);
    }
}

void MainWindow::SetTask(qreal task)
{
    quint16 i_task = qRound(task * 1000);

    if (ui->doubleSpinBox_task->value() != i_task / 1000.0) {
        ui->doubleSpinBox_task->setValue(i_task / 1000.0);
    }

    if (ui->verticalSlider_task->value() != i_task) {
        ui->verticalSlider_task->setSliderPosition(i_task);
    }
}

void MainWindow::SetTextColor(TextObjects object, const QColor color)
{
    if (m_labels.contains(object)) {
        m_labels[object]->setStyleSheet("color:" + color.name(QColor::HexRgb));
    }
}

void MainWindow::SetStepTestResults(QVector<StepTest::TestResult> results, quint32 T_value)
{
    ui->tableWidget_stepResults->setHorizontalHeaderLabels(
        {QString("T%1").arg(T_value), "Перерегулирование"});

    ui->tableWidget_stepResults->setRowCount(results.size());
    QStringList rowNames;
    for (int i = 0; i < results.size(); ++i) {

        QString time = results.at(i).T_value == 0
                           ? "Ошибка"
                           : QTime(0, 0).addMSecs(results.at(i).T_value).toString("m:ss.zzz");

        ui->tableWidget_stepResults->setItem(i, 0, new QTableWidgetItem(time));

        QString overshoot = QString("%1%").arg(results.at(i).overshoot, 4, 'f', 2);
        ui->tableWidget_stepResults->setItem(i, 1, new QTableWidgetItem(overshoot));

        QString rowName = QString("%1-%2").arg(results.at(i).from).arg(results.at(i).to);
        rowNames << rowName;
    }
    ui->tableWidget_stepResults->setVerticalHeaderLabels(rowNames);
    ui->tableWidget_stepResults->resizeColumnsToContents();

    m_telemetryStore.stepResults.clear();

    for (const auto &r : results) {
        StepTestRecord rec;

        rec.from = r.from;
        rec.to = r.from;
        rec.T_value = r.T_value;
        rec.overshoot = r.overshoot;

        m_telemetryStore.stepResults.push_back(rec);
    }
}

void MainWindow::SetSolenoidResults(QString sequence, quint16 cycles, double totalTimeSec)
{
    m_lastSolenoidSequence = sequence;

    qint64 totalMs = qRound(totalTimeSec * 1000.0);
    QTime t(0, 0);
    t = t.addMSecs(totalMs);

    ui->lineEdit_cyclicTest_sequence->setText(sequence);
    ui->lineEdit_cyclicTest_totalTime->setText(
        t.toString("hh:mm:ss.zzz"));
    ui->lineEdit_cyclicTest_cycles->setText(
        QString::number(cycles));
}

void MainWindow::onSolenoidRangesData(const QVector<RangeDeviationRecord>& ranges)
{
    m_telemetryStore.cyclicTestRecord.ranges = ranges;
}

void MainWindow::ВideTabByWidget(QWidget *page) {
    int idx = ui->tabWidget->indexOf(page);
    if (idx >= 0) ui->tabWidget->setTabVisible(idx, false);
}

void MainWindow::DisplayDependingPattern() {
    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT:
        ui->groupBox_DO->setVisible(false);

        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        break;
    case SelectTests::Pattern_B_SACVT:
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        break;
    case SelectTests::Pattern_C_CVT:
        ui->groupBox_DO->setVisible(false);
        break;
    case SelectTests::Pattern_C_SACVT:
        break;
    case SelectTests::Pattern_C_SOVT:
        ui->groupBox_SettingCurrentSignal->setVisible(false);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        break;
    default:
        break;
    }
}

void MainWindow::SetSensorsNumber(quint8 num)
{
    bool noSensors = (num == 0);

    if (!m_blockCTS.moving) {
        ui->label_startingPositionValue->setVisible(false);
        // ui->label_finalPositionValue->setVisible(false);
        ui->label_startingPosition->setVisible(false);
        // ui->label_finalPosition->setVisible(false);
    }

    ui->groupBox_SettingCurrentSignal->setEnabled(!noSensors);

    ui->pushButton_mainTest_start->setEnabled(num > 1);
    ui->pushButton_strokeTest_start->setEnabled(!noSensors);
    ui->pushButton_optionalTests_start->setEnabled(!noSensors);
    ui->pushButton_cyclicTest_start->setEnabled(!noSensors);

    ui->doubleSpinBox_task->setEnabled(!noSensors);
    ui->verticalSlider_task->setEnabled(!noSensors);

    if (num > 0) {
        ui->checkBox_showCurve_task->setVisible(num > 1);
        ui->checkBox_showCurve_moving->setVisible(num > 1);
        ui->checkBox_showCurve_pressure_1->setVisible(num > 1);
        ui->checkBox_showCurve_pressure_2->setVisible(num > 2);
        ui->checkBox_showCurve_pressure_3->setVisible(num > 3);

        ui->checkBox_showCurve_task->setCheckState(num > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_moving->setCheckState(num > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_pressure_1->setCheckState(num > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_pressure_2->setCheckState(num > 2 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_showCurve_pressure_3->setCheckState(num > 3 ? Qt::Checked : Qt::Unchecked);
    }
}
void MainWindow::SetButtonInitEnabled(bool enable)
{
    ui->pushButton_init->setEnabled(enable);
}

void MainWindow::AddPoints(Charts chart, QVector<Point> points)
{
    for (const auto& point : points)
        m_charts[chart]->addPoint(point.series_num, point.X, point.Y);
}

void MainWindow::ClearPoints(Charts chart)
{
    m_charts[chart]->clear();
}

void MainWindow::SetChartVisible(Charts chart, quint16 series, bool visible)
{
    m_charts[chart]->visible(series, visible);
}

void MainWindow::ShowDots(bool visible)
{
    m_charts[Charts::Task]->showdots(visible);
    m_charts[Charts::Pressure]->showdots(visible);
}

void MainWindow::DublSeries()
{
    m_charts[Charts::Task]->dublSeries(1);
    m_charts[Charts::Task]->dublSeries(2);
    m_charts[Charts::Task]->dublSeries(3);
    m_charts[Charts::Task]->dublSeries(4);
    m_charts[Charts::Pressure]->dublSeries(0);
}

void MainWindow::EnableSetTask(bool enable)
{
    ui->verticalSlider_task->setEnabled(enable);
    ui->doubleSpinBox_task->setEnabled(enable);
}

void MainWindow::GetPoints(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();
    if (chart == Charts::Task) {
        QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[Charts::Task]->getPoints(1);

        QPair<QList<QPointF>, QList<QPointF>> pointsPressure = m_charts[Charts::Pressure]->getPoints(0);

        points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
        points.push_back({pointsLinear.second.begin(), pointsLinear.second.end()});
        points.push_back({pointsPressure.first.begin(), pointsPressure.first.end()});
        points.push_back({pointsPressure.second.begin(), pointsPressure.second.end()});
    }
    if (chart == Charts::Step) {
        QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[Charts::Step]->getPoints(1);
        QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_charts[Charts::Step]->getPoints(0);

        points.clear();
        points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
        points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
    }
}

void MainWindow::SetRegressionEnable(bool enable)
{
    ui->checkBox_regression->setEnabled(enable);
    ui->checkBox_regression->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::GetMainTestParameters(MainTestSettings::TestParameters &parameters)
{
    if (m_mainTestSettings->exec() == QDialog::Accepted) {
        parameters = m_mainTestSettings->getParameters();

        qint64 runMs = m_mainTestSettings->totalTestTimeMillis();

        const qint64 delayMs = 15 * 1000;
        const qint64 delayMs2 = 10 * 1000;

        m_totalTestMs = delayMs
                        + runMs
                        + delayMs2
                        + runMs;

        m_elapsedTimer.start();
        m_durationTimer->start();
        QTime t(0, 0);
        t = t.addMSecs(m_totalTestMs);
        ui->lineEdit_testDuration->setText(t.toString("hh:mm:ss.zzz"));

        return;
    }

    parameters.delay = 0;
    return;
}

void MainWindow::GetStepTestParameters(StepTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_stepTestSettings->exec() == QDialog::Accepted) {
        parameters = m_stepTestSettings->getParameters();
    }
}

void MainWindow::GetCyclicTestParameters(CyclicTestSettings::TestParameters &parameters)
{
    if (m_cyclicTestSettings->exec() == QDialog::Accepted) {
        parameters = m_cyclicTestSettings->getParameters();
    }
    else {
        parameters = {};
    }
}

void MainWindow::GetResolutionTestParameters(OtherTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_resolutionTestSettings->exec() == QDialog::Accepted) {
        parameters = m_resolutionTestSettings->getParameters();
    }
}

void MainWindow::GetResponseTestParameters(OtherTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_responseTestSettings->exec() == QDialog::Accepted) {
        parameters = m_responseTestSettings->getParameters();
    }
}

void MainWindow::Question(QString title, QString text, bool &result)
{
    result = (QMessageBox::question(NULL, title, text) == QMessageBox::Yes);
}

void MainWindow::GetDirectory(QString current_path, QString &result)
{
    result = QFileDialog::getExistingDirectory(this,
                                               "Выберите папку для сохранения изображений",
                                               current_path);
}

void MainWindow::StartTest()
{
    m_testing = true;
    ui->statusbar->showMessage("Тест в процессе");
}

void MainWindow::EndTest()
{
    m_testing = false;
    ui->statusbar->showMessage("Тест завершен");
}

void MainWindow::ButtonStartMain()
{
    if (m_testing) {
        if (QMessageBox::question(this, "Внимание!", "Вы действительно хотите завершить тест?")
            == QMessageBox::Yes) {
            emit StopTest();
        }
    } else {

        emit StartMainTest();
        StartTest();
    }
}

void MainWindow::ButtonStartStroke()
{
    if (m_testing) {
        if (QMessageBox::question(this, "Внимание!", "Вы действительно хотите завершить тест?")
            == QMessageBox::Yes) {
            emit StopTest();
        }
    } else {

        emit StartStrokeTest();
        StartTest();
    }
}

void MainWindow::ButtonStartOptional()
{
    if (m_testing) {
        if (QMessageBox::question(this, "Внимание!", "Вы действительно хотите завершить тест?")
            == QMessageBox::Yes) {
            emit StopTest();
        }
    } else {

        emit StartOptionalTest(ui->tabWidget_tests->currentIndex());
        StartTest();
    }
}

void MainWindow::ButtonStartCyclicSolenoid() {
    qDebug() << "[MW] ButtonStartCyclicSolenoid called; m_testing =" << m_testing;

    if (m_testing) {
        if (QMessageBox::question(this, "Внимание!", "Вы действительно хотите завершить тест?")
            == QMessageBox::Yes) {
            emit StopTest();
        }
    } else {

        CyclicTestSettings::AvailableTests avail;
        using PT = SelectTests::PatternType;
        if (m_patternType == PT::Pattern_B_CVT || m_patternType == PT::Pattern_C_CVT) {
            avail = CyclicTestSettings::OnlyRegulatory;
        }
        else if (m_patternType == PT::Pattern_C_SOVT) {
            avail = CyclicTestSettings::OnlyShutoff;
        }
        else if (m_patternType == PT::Pattern_B_SACVT || m_patternType == PT::Pattern_C_SACVT) {
            avail = CyclicTestSettings::ZipRegulatory;
        } else {
            avail = CyclicTestSettings::ZipRegulatory;
        }

        m_cyclicTestSettings->setAvailableTests(avail);
        if (m_cyclicTestSettings->exec() != QDialog::Accepted)
            return;

        using TP = CyclicTestSettings::TestParameters;
        auto p = m_cyclicTestSettings->getParameters();

        if ((p.testType == TP::Shutoff || p.testType == TP::Combined)
            &&  p.shutoff_enable_20mA)
        {
            emit SetDAC(20.0);
        }

        auto countSteps = [](const QString& seq){
            return seq.split('-', Qt::SkipEmptyParts).size();
        };
        qint64 totalMs = 0;
        if (p.testType == CyclicTestSettings::TestParameters::Regulatory ||
            p.testType == CyclicTestSettings::TestParameters::Combined)
        {
            int steps = countSteps(p.regulatory_sequence);
            totalMs += qint64(steps)
                       * p.regulatory_numCycles
                       * (p.regulatory_delaySec + p.regulatory_holdTimeSec)
                       * 1000;
        }
        if (p.testType == CyclicTestSettings::TestParameters::Shutoff ||
            p.testType == CyclicTestSettings::TestParameters::Combined)
        {
            int steps = countSteps(p.shutoff_sequence);
            totalMs += qint64(steps)
                       * p.shutoff_numCycles
                       * (p.shutoff_delaySec + p.shutoff_holdTimeSec)
                       * 1000;
        }

        QTime t0(0, 0);
        t0 = t0.addMSecs(totalMs);
        ui->lineEdit_cyclicTest_totalTime->setText(
            t0.toString("hh:mm:ss.zzz"));

        m_cyclicTotalMs = totalMs;
        m_cyclicElapsedTimer.restart();
        m_cyclicCountdownTimer.start();

        StartTest();
        emit StartCyclicSolenoidTest(p);
    }
}

void MainWindow::SetButtonsDOChecked(quint8 status)
{
    ui->pushButton_DO0->blockSignals(true);
    ui->pushButton_DO1->blockSignals(true);
    ui->pushButton_DO2->blockSignals(true);
    ui->pushButton_DO3->blockSignals(true);

    ui->pushButton_DO0->setChecked((status & (1 << 0)) != 0);
    ui->pushButton_DO1->setChecked((status & (1 << 1)) != 0);
    ui->pushButton_DO2->setChecked((status & (1 << 2)) != 0);
    ui->pushButton_DO3->setChecked((status & (1 << 3)) != 0);

    ui->pushButton_DO0->blockSignals(false);
    ui->pushButton_DO1->blockSignals(false);
    ui->pushButton_DO2->blockSignals(false);
    ui->pushButton_DO3->blockSignals(false);
}

void MainWindow::SetCheckboxDIChecked(quint8 status)
{
    ui->checkBox_switch_3_0->setChecked((status & (1 << 0)) != 0);
    ui->checkBox_switch_0_3->setChecked((status & (1 << 1)) != 0);
}

void MainWindow::InitCharts()
{
    ValveInfo *valveInfo = m_registry->GetValveInfo();
    bool rotate = (valveInfo->strokeMovement != 0);

    m_charts[Charts::Task] = ui->Chart_task;
    m_charts[Charts::Task]->setName("Task");
    m_charts[Charts::Task]->useTimeaxis(false);
    m_charts[Charts::Task]->addAxis("%.2f bar");
    if (!rotate) {
        m_charts[Charts::Task]->addAxis("%.2f mm");
    } else {
        m_charts[Charts::Task]->addAxis("%.2f deg");
    }
    m_charts[Charts::Task]->addSeries(1, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Task]->addSeries(1,"Датчик линейных перемещений",QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Task]->addSeries(0, "Датчик давления 1", QColor::fromRgb(0, 0, 255));
    m_charts[Charts::Task]->addSeries(0, "Датчик давления 2", QColor::fromRgb(0, 200, 0));
    m_charts[Charts::Task]->addSeries(0, "Датчик давления 3", QColor::fromRgb(150, 0, 200));


    m_charts[Charts::Friction] = ui->Chart_friction;
    m_charts[Charts::Friction]->setName("Friction");
    m_charts[Charts::Friction]->addAxis("%.2f H");
    m_charts[Charts::Friction]->addSeries(0, "Трение от перемещения", QColor::fromRgb(255, 0, 0));
    if (!rotate) {
        m_charts[Charts::Friction]->setLabelXformat("%.2f mm");
    } else {
        m_charts[Charts::Friction]->setLabelXformat("%.2f deg");
    }


    m_charts[Charts::Pressure] = ui->Chart_pressure;
    m_charts[Charts::Pressure]->setName("Pressure");
    m_charts[Charts::Pressure]->useTimeaxis(false);
    m_charts[Charts::Pressure]->setLabelXformat("%.2f bar");
    if (!rotate) {
        m_charts[Charts::Pressure]->addAxis("%.2f mm");
    } else {
        m_charts[Charts::Pressure]->addAxis("%.2f deg");
    }
    m_charts[Charts::Pressure]->addSeries(0, "Перемещение от давления", QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Pressure]->addSeries(0, "Линейная регрессия", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Pressure]->visible(1, false);

    m_charts[Charts::Resolution] = ui->Chart_resolution;
    m_charts[Charts::Resolution]->setName("Resolution");
    m_charts[Charts::Resolution]->useTimeaxis(true);
    m_charts[Charts::Resolution]->addAxis("%.2f%%");
    m_charts[Charts::Resolution]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Resolution]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));

    m_charts[Charts::Response] = ui->Chart_response;
    m_charts[Charts::Response]->setName("Response");
    m_charts[Charts::Response]->useTimeaxis(true);
    m_charts[Charts::Response]->addAxis("%.2f%%");
    m_charts[Charts::Response]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Response]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));

    m_charts[Charts::Stroke] = ui->Chart_stroke;
    m_charts[Charts::Stroke]->setName("Stroke");
    m_charts[Charts::Stroke]->useTimeaxis(true);
    m_charts[Charts::Stroke]->addAxis("%.2f%%");
    m_charts[Charts::Stroke]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Stroke]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));

    m_charts[Charts::Step] = ui->Chart_step;
    m_charts[Charts::Step]->setName("Step");
    m_charts[Charts::Step]->useTimeaxis(true);
    m_charts[Charts::Step]->addAxis("%.2f%%");
    m_charts[Charts::Step]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Step]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));

    m_charts[Charts::Trend] = ui->Chart_trend;
    m_charts[Charts::Trend]->useTimeaxis(true);
    m_charts[Charts::Trend]->addAxis("%.2f%%");
    m_charts[Charts::Trend]->addAxis("%.2f bar");
    m_charts[Charts::Trend]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Trend]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Trend]->setMaxRange(60000);

    m_charts[Charts::CyclicSolenoid] = ui->Chart_cyclicSolenoid;
    m_charts[Charts::CyclicSolenoid]->setName("Cyclic_solenoid");
    m_charts[Charts::CyclicSolenoid]->useTimeaxis(true);
    m_charts[Charts::CyclicSolenoid]->addAxis("%.2f%%");
    m_charts[Charts::CyclicSolenoid]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::CyclicSolenoid]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));
    // m_charts[Charts::CyclicSolenoid]->setMaxRange(160000);

    if (m_patternType == SelectTests::Pattern_C_SOVT || m_patternType == SelectTests::Pattern_B_SACVT || m_patternType ==  SelectTests::Pattern_C_SACVT) {
        m_charts[Charts::CyclicSolenoid]->addSeries(0, "DI1", QColor::fromRgb(200, 200, 0));
        m_charts[Charts::CyclicSolenoid]->addSeries(0, "DI2", QColor::fromRgb(0, 200, 0));

        m_charts[Charts::CyclicSolenoid]->setPointsVisible(2, true);
        m_charts[Charts::CyclicSolenoid]->setPointsVisible(3, true);
    }

    connect(m_program, &Program::AddPoints, this, &MainWindow::AddPoints);
    connect(m_program, &Program::ClearPoints, this, &MainWindow::ClearPoints);
    connect(m_program, &Program::DublSeries, this, &MainWindow::DublSeries);
    connect(m_program, &Program::SetVisible, this, &MainWindow::SetChartVisible);
    connect(m_program, &Program::SetRegressionEnable, this, &MainWindow::SetRegressionEnable);
    connect(m_program, &Program::ShowDots, this, &MainWindow::ShowDots);

    connect(ui->checkBox_showCurve_task, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(0, k != 0);
    });

    connect(ui->checkBox_showCurve_moving, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(1, k != 0);
    });

    connect(ui->checkBox_showCurve_pressure_1, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(2, k != 0);
    });

    connect(ui->checkBox_showCurve_pressure_2, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(3, k != 0);
    });

    connect(ui->checkBox_showCurve_pressure_3, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(4, k != 0);
    });

    connect(ui->checkBox_regression, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Pressure]->visible(1, k != 0);
    });

    connect(m_program,
            &Program::GetPoints,
            this,
            &MainWindow::GetPoints,
            Qt::BlockingQueuedConnection);

    // ui->checkBox_showCurve_task->setCheckState(Qt::Unchecked);
    // ui->checkBox_showCurve_moving->setCheckState(Qt::Unchecked);
    // ui->checkBox_showCurve_pressure_1->setCheckState(Qt::Unchecked);
    // ui->checkBox_showCurve_pressure_2->setCheckState(Qt::Unchecked);
    // ui->checkBox_showCurve_pressure_3->setCheckState(Qt::Unchecked);
}

void MainWindow::promptSaveCharts()
{
    auto answer = QMessageBox::question(
        this,
        tr("Сохранение результатов"),
        tr("Тест MainTest завершён.\nСохранитаь графики Task, Pressure и Friction?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (answer == QMessageBox::Yes) {
        SaveChart(Charts::Task);
        SaveChart(Charts::Pressure);
        SaveChart(Charts::Friction);
        QMessageBox::information(
            this,
            tr("Готово"),
            tr("Графики сохранены в текущую папку отчётов.")
            );
    }
}

void MainWindow::SaveChart(Charts chart)
{
    m_reportSaver->SaveImage(m_charts[chart]);

    QPixmap pix = m_charts[chart]->grab();

    QImage img = pix.toImage();

    switch (chart) {
    case Charts::Task:
        ui->label_imageChartTask->setPixmap(pix);
        m_imageChartTask = img;
        break;
    case Charts::Stroke:
        break;
    case Charts::Response:
        break;
    case Charts::Resolution:
        break;
    case Charts::Step:
        m_imageChartStep = img;
        break;
    case Charts::Pressure:
        ui->label_imageChartPressure->setPixmap(pix);
        m_imageChartPressure = img;
        break;
    case Charts::Friction:
        ui->label_imageChartFriction->setPixmap(pix);
        m_imageChartFriction = img;
        break;
    case Charts::Trend:
        break;
    case Charts::CyclicSolenoid:
        break;
    default:
        break;
    }

    ui->label_imageChartTask->setScaledContents(true);
    ui->label_imageChartPressure->setScaledContents(true);
    ui->label_imageChartFriction->setScaledContents(true);
}

void MainWindow::GetImage(QLabel *label, QImage *image)
{
    QString imgPath = QFileDialog::getOpenFileName(this,
                                                   "Выберите файл",
                                                   m_reportSaver->Directory().absolutePath(),
                                                   "Изображения (*.jpg *.png *.bmp)");

    if (!imgPath.isEmpty()) {
        QImage img(imgPath);
        *image = img.scaled(1000, 430, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(QPixmap::fromImage(img));
    }
}

void MainWindow::collectTestTelemetryData() {
    // динамика
    // m_telemetry.dinamicRecord.dinamicReal = ui->lineEdit_dinamicReal->text();
    // m_telemetry.dinamicRecord.dinamicRecomend= ui->lineEdit_dinamicRecomend->text();
    // m_telemetry.dinamicRecord.dinamicIpReal = ui->lineEdit_dinamicIpReal->text();
    // m_telemetry.dinamicRecord.dinamicIpRecomend = ui->lineEdit_dinamicIpRecomend->text();

    // // ход клапана
    // m_telemetry.strokeTestRecord.timeForward = ui->lineEdit_strokeTest_forwardTime->text();
    // m_telemetry.strokeTestRecord.timeBackward= ui->lineEdit_strokeTest_backwardTime->text();

    // // Циклический тест
    // m_telemetry.cyclicTestRecord.sequence = ui->lineEdit_cyclicTest_sequence->text();
    // m_telemetry.cyclicTestRecord.cycles = ui->lineEdit_cyclicTest_cycles->text();
    // m_telemetry.cyclicTestRecord.totalTime = ui->lineEdit_cyclicTest_totalTime->text();

    // // ход штока / вала
    // m_telemetry.strokeRecord.strokeReal = ui->lineEdit_strokeReal->text();
    // m_telemetry.strokeRecord.strokeRecomend = ui->lineEdit_strokeRecomend->text();

    // // m_telemetry.data.push_back({30, 5, ui->lineEdit_strokeReal});
    // // m_telemetry.data.push_back({30, 8, ui->lineEdit_strokeRecomend});

    // // диапазон
    // m_telemetry.rangeRecord.rangeReal = ui->lineEdit_rangeReal->text();
    // m_telemetry.rangeRecord.rangeRecomend= ui->lineEdit_rangeRecomend->text();
    // m_telemetry.rangeRecord.rangePressure= ui->lineEdit_rangePressure->text();

    // // трение
    // m_telemetry.frictionRecord.friction  = ui->lineEdit_friction->text();
    // m_telemetry.frictionRecord.frictionPercent = ui->lineEdit_frictionPercent->text();

    // // подача
    // m_telemetry.supplyRecord.supplyPressure = ui->lineEdit_supplyPressure->text();
}
