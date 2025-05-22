#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CyclicTestSettings.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    m_testing = false;



    m_mainTestSettings = new MainTestSettings(this);
    m_stepTestSettings = new StepTestSettings(this);
    m_responseTestSettings = new OtherTestSettings(this);
    m_resolutionTestSettings = new OtherTestSettings(this);
    m_cyclicTestSettings = new CyclicTestSettings(this);

    m_reportSaver = new ReportSaver(this);

    ui->groupBox_DO->setVisible(false);

    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tab_main), false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);

    ui->checkBox_DI1->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_DI1->setFocusPolicy(Qt::NoFocus);
    ui->checkBox_DI2->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_DI2->setFocusPolicy(Qt::NoFocus);

    m_labels[TextObjects::Label_status] = ui->label_status;
    m_labels[TextObjects::Label_init] = ui->label_init;
    m_labels[TextObjects::Label_sensors] = ui->label_sensors;
    m_labels[TextObjects::Label_start_value] = ui->label_start_value;
    m_labels[TextObjects::Label_end_value] = ui->label_end_value;
    m_labels[TextObjects::Label_L_limit] = ui->label_low_limit;
    m_labels[TextObjects::Label_H_limit] = ui->label_high_limit;
    m_labels[TextObjects::Label_pressure_diff] = ui->label_pressure_diff;
    m_labels[TextObjects::Label_friction] = ui->label_friction;
    m_labels[TextObjects::Label_friction_percent] = ui->label_friction_percent;
    m_labels[TextObjects::Label_din_error_mean] = ui->label_din_error_mean;
    m_labels[TextObjects::Label_din_error_mean_percent] = ui->label_din_error_mean_percent;
    m_labels[TextObjects::Label_din_error_max] = ui->label_din_error_max;
    m_labels[TextObjects::Label_din_error_max_percent] = ui->label_din_error_max_percent;
    m_labels[TextObjects::Label_range] = ui->label_range;
    m_labels[TextObjects::Label_low_limit] = ui->label_low_limit;
    m_labels[TextObjects::Label_high_limit] = ui->label_high_limit;
    m_labels[TextObjects::Label_forward] = ui->label_forward;
    m_labels[TextObjects::Label_backward] = ui->label_backward;

    m_lineEdits[TextObjects::LineEdit_linear_sensor] = ui->lineEdit_linear_sensor;
    m_lineEdits[TextObjects::LineEdit_linear_sensor_percent] = ui->lineEdit_linear_sensor_percent;
    m_lineEdits[TextObjects::LineEdit_pressure_sensor1] = ui->lineEdit_pressure_sensor1;
    m_lineEdits[TextObjects::LineEdit_pressure_sensor2] = ui->lineEdit_pressure_sensor2;
    m_lineEdits[TextObjects::LineEdit_pressure_sensor3] = ui->lineEdit_pressure_sensor3;
    m_lineEdits[TextObjects::LineEdit_feedback_4_20mA] = ui->lineEdit_feedback_4_20mA;
    m_lineEdits[TextObjects::LineEdit_dinamic_error] = ui->lineEdit_dinamic_real;
    m_lineEdits[TextObjects::LineEdit_stroke] = ui->lineEdit_stroke_real;
    m_lineEdits[TextObjects::LineEdit_range] = ui->lineEdit_range_real;
    m_lineEdits[TextObjects::LineEdit_friction] = ui->lineEdit_friction;
    m_lineEdits[TextObjects::LineEdit_friction_percent] = ui->lineEdit_friction_percent;
    m_lineEdits[TextObjects::LineEdit_forward] = ui->lineEdit_time_forward;
    m_lineEdits[TextObjects::LineEdit_backward] = ui->lineEdit_time_backward;
    m_lineEdits[TextObjects::LineEdit_range_pressure] = ui->lineEdit_range_pressure;

    m_program = new Program;
    m_programthread = new QThread(this);

    m_program->moveToThread(m_programthread);

    connect(ui->pushButton_init, &QPushButton::clicked, m_program, &Program::button_init);
    connect(ui->pushButton_set, &QPushButton::clicked, m_program, &Program::button_set_position);
    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged, m_program, &Program::checkbox_autoinit);

    connect(this, &MainWindow::SetDO, m_program, &Program::button_DO);

    for (int i = 0; i < 4; ++i) {
        auto btn = findChild<QPushButton*>(QString("pushButton_DO%1").arg(i));
        if (!btn) continue;
        connect(btn, &QPushButton::clicked, this, [this, i](bool checked){
            emit SetDO(i, checked);
        });
    }

    connect(ui->pushButton_main_save, &QPushButton::clicked, this, [&] {
        if (ui->tabWidget_maintest->currentWidget() == ui->tab_task) {
            SaveChart(Charts::Task);
        } else if (ui->tabWidget_maintest->currentWidget() == ui->tab_pressure) {
            SaveChart(Charts::Pressure);
        } else if (ui->tabWidget_maintest->currentWidget() == ui->tab_friction) {
            SaveChart(Charts::Friction);
        }
    });

    connect(ui->pushButton_main_start,
            &QPushButton::clicked,
            this,
            &MainWindow::ButtonStartMain);

    connect(ui->pushButton_stroke_start,
            &QPushButton::clicked,
            this,
            &MainWindow::ButtonStartStroke);
    connect(ui->pushButton_tests_start,
            &QPushButton::clicked,
            this,
            &MainWindow::ButtonStartOptional);

    connect(
        this,
        &MainWindow::StartCyclicSolenoidTest,
        m_program,
        &Program::CyclicSolenoidTestStart
    );

    connect(ui->pushButton_stroke_save, &QPushButton::clicked, this, [&] {
        SaveChart(Charts::Stroke);
    });

    connect(ui->pushButton_tests_save, &QPushButton::clicked, this, [&] {
        if (ui->tabWidget_tests->currentWidget() == ui->tab_response) {
            SaveChart(Charts::Response);
        } else if (ui->tabWidget_tests->currentWidget() == ui->tab_resolution) {
            SaveChart(Charts::Resolution);
        } else if (ui->tabWidget_tests->currentWidget() == ui->tab_step) {
            SaveChart(Charts::Step);
        }
    });

    connect(ui->pushButton_cyclic_solenoid_save, &QPushButton::clicked, this, [&](){
        SaveChart(Charts::CyclicSolenoid);
    });

    connect(ui->pushButton_open, &QPushButton::clicked, m_program, &Program::button_open);
    connect(ui->pushButton_report, &QPushButton::clicked, m_program, &Program::button_report);
    connect(ui->pushButton_pixmap1, &QPushButton::clicked, m_program, &Program::button_pixmap1);
    connect(ui->pushButton_pixmap2, &QPushButton::clicked, m_program, &Program::button_pixmap2);
    connect(ui->pushButton_pixmap3, &QPushButton::clicked, m_program, &Program::button_pixmap3);

    connect(this, &MainWindow::StartMainTest, m_program, &Program::MainTestStart);
    connect(this, &MainWindow::StartStrokeTest, m_program, &Program::StrokeTestStart);
    connect(this, &MainWindow::StartOptionalTest, m_program, &Program::StartOptionalTest);
    connect(this, &MainWindow::StopTest, m_program, &Program::TerminateTest);

    connect(m_program, &Program::StopTest, this, &MainWindow::EndTest);

    connect(m_program, &Program::SetText, this, &MainWindow::SetText);
    connect(m_program, &Program::SetTextColor, this, &MainWindow::SetTextColor);
    connect(m_program, &Program::SetSolenoidResults, this, &MainWindow::SetSolenoidResults);


    connect(m_program, &Program::SetGroupDOVisible, this, [&](bool visible) {
        ui->groupBox_DO->setVisible(visible);
    });

    connect(m_program, &Program::SetButtonsDOChecked, this, &MainWindow::SetButtonsDOChecked);
    connect(m_program, &Program::SetCheckboxDIChecked, this, &MainWindow::SetCheckboxDIChecked);

    connect(this, &MainWindow::SetDAC, m_program, &Program::SetDAC_real);

    connect(ui->doubleSpinBox_task,
        qOverload<double>(&QDoubleSpinBox::valueChanged),
        this,
        [&](double value) {
            if (qRound(value * 1000) != ui->verticalSlider_task->value()) {
                if (ui->verticalSlider_task->isEnabled())
                    emit SetDAC(value);
                ui->verticalSlider_task->setValue(qRound(value * 1000));
            }
        }
    );

    connect(ui->verticalSlider_task, &QSlider::valueChanged, this, [&](int value) {
        if (qRound(ui->doubleSpinBox_task->value() * 1000) != value) {
            if (ui->doubleSpinBox_task->isEnabled())
                emit SetDAC(value / 1000.0);
            ui->doubleSpinBox_task->setValue(value / 1000.0);
        }
    });

    connect(m_program, &Program::SetTask, this, &MainWindow::SetTask);
    connect(m_program, &Program::SetSensorNumber, this, [=](quint8 num) {
        SetSensorsNumber(num);
    });    connect(m_program, &Program::SetButtonInitEnabled, this, &MainWindow::SetButtonInitEnabled);
    connect(m_program, &Program::EnableSetTask, this, &MainWindow::EnableSetTask);
    connect(m_program, &Program::SetStepResults, this, &MainWindow::SetStepTestResults);
    connect(m_program,
            &Program::GetMainTestParameters,
            this,
            &MainWindow::GetMainTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program,
            &Program::GetStepTestParameters,
            this,
            &MainWindow::GetStepTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program,
            &Program::GetResolutionTestParameters,
            this,
            &MainWindow::GetResolutionTestParameters,
            Qt::BlockingQueuedConnection);

    connect(m_program,
            &Program::GetResponseTestParameters,
            this,
            &MainWindow::GetResponseTestParameters,
            Qt::BlockingQueuedConnection);


    connect(m_program,
            &Program::GetCyclicTestParameters,
            this,
            &MainWindow::GetCyclicTestParameters,
            Qt::BlockingQueuedConnection);

    connect(ui->pushButton_cyclicSolenoidStart, &QPushButton::clicked,
            this, &MainWindow::ButtonStartCyclicSolenoid);

    connect(m_program, &Program::Question, this, &MainWindow::Question, Qt::BlockingQueuedConnection);

    connect(m_reportSaver, &ReportSaver::Question, this, &MainWindow::Question, Qt::DirectConnection);
    connect(m_reportSaver,
            &ReportSaver::GetDirectory,
            this,
            &MainWindow::GetDirectory,
            Qt::DirectConnection);

    connect(ui->pushButton_pixmap1, &QPushButton::clicked, this, [&] {
        GetImage(ui->label_pixmap1, &m_report.image1);
    });

    connect(ui->pushButton_pixmap2, &QPushButton::clicked, this, [&] {
        GetImage(ui->label_pixmap2, &m_report.image2);
    });

    connect(ui->pushButton_pixmap3, &QPushButton::clicked, this, [&] {
        GetImage(ui->label_pixmap3, &m_report.image3);
    });

    connect(ui->pushButton_report, &QPushButton::clicked, this, [&] {
        ui->pushButton_open->setEnabled(m_reportSaver->SaveReport(m_report));
    });

    connect(ui->pushButton_open, &QPushButton::clicked, this, [&] {
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(m_reportSaver->Directory().filePath("report.xlsx")));
    });

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged, this, [&](int state) {
        ui->pushButton_set->setEnabled(!state);
    });

    InitReport();
    // SetSensorsNumber(0);

    ui->tableWidget_step_results->setColumnCount(2);
    ui->tableWidget_step_results->setHorizontalHeaderLabels({"T86", "Перерегулирование"});
    ui->tableWidget_step_results->resizeColumnsToContents();

    connect(m_program, &Program::SetSolenoidResults, this, &MainWindow::SetSolenoidResults);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_programthread->quit();
    m_programthread->wait();
}

void MainWindow::SetRegistry(Registry *registry)
{
    m_registry = registry;

    ObjectInfo *objectInfo = m_registry->GetObjectInfo();
    ValveInfo *valveInfo = m_registry->GetValveInfo();
    OtherParameters *otherParameters = m_registry->GetOtherParameters();

    ui->lineEdit_date->setText(otherParameters->date);
    ui->lineEdit_strokeMovement->setText(otherParameters->strokeMovement);

    ui->lineEdit_object->setText(objectInfo->object);
    ui->lineEdit_manufacture->setText(objectInfo->manufactory);
    ui->lineEdit_department->setText(objectInfo->department);
    ui->lineEdit_FIO->setText(objectInfo->FIO);

    ui->lineEdit_positionNumber->setText(valveInfo->positionNumber);
    ui->lineEdit_manufacturer->setText(valveInfo->manufacturer);
    ui->lineEdit_serialNumber->setText(valveInfo->serialNumber);
    ui->lineEdit_DNPN->setText(valveInfo->DN + "/" + valveInfo->PN);
    ui->lineEdit_positionerModel->setText(valveInfo->positionerModel);
    ui->lineEdit_dinamic_recomend->setText(QString::number(valveInfo->dinamicError, 'f', 2));
    ui->lineEdit_stroke_recomend->setText(valveInfo->strokValve);
    ui->lineEdit_range_recomend->setText(valveInfo->driveRange);
    ui->lineEdit_valveModel->setText(valveInfo->valveModel);

    if (valveInfo->safePosition != 0) {
        m_stepTestSettings->reverse();
        m_responseTestSettings->reverse();
        m_resolutionTestSettings->reverse();
    }

    InitCharts();

    m_program->SetRegistry(registry);
    m_programthread->start();

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
    ui->tableWidget_step_results->setHorizontalHeaderLabels(
        {QString("T%1").arg(T_value), "Перерегулирование"});

    ui->tableWidget_step_results->setRowCount(results.size());
    QStringList rowNames;
    for (int i = 0; i < results.size(); ++i) {

        QString time = results.at(i).T_value == 0
            ? "Ошибка"
            : QTime(0, 0).addMSecs(results.at(i).T_value).toString("m:ss.zzz");

        ui->tableWidget_step_results->setItem(i, 0, new QTableWidgetItem(time));
        QString overshoot = QString("%1%").arg(results.at(i).overshoot, 4, 'f', 2);
        ui->tableWidget_step_results->setItem(i, 1, new QTableWidgetItem(overshoot));
        QString rowName = QString("%1-%2").arg(results.at(i).from).arg(results.at(i).to);
        rowNames << rowName;
    }
    ui->tableWidget_step_results->setVerticalHeaderLabels(rowNames);
    ui->tableWidget_step_results->resizeColumnsToContents();
}

void MainWindow::SetSolenoidResults(double forwardSec, double backwardSec, quint16 cycles, double rangePercent,  double totalTimeSec)
{
    QString forwardText = QTime(0, 0).addMSecs(forwardSec).toString("mm:ss.zzz");
    QString backwardText = QTime(0, 0).addMSecs(backwardSec).toString("mm:ss.zzz");
    ui->lineEdit_forwardSec->setText(QString::number(forwardSec, 'f', 2));
    ui->lineEdit_backwardSec->setText(QString::number(backwardSec, 'f', 2));
    ui->lineEdit_rangePercent->setText(QString::number(rangePercent, 'f', 1));
    ui->lineEdit_totalTimeSec->setText(QString::number(totalTimeSec, 'f', 1));
    ui->lineEdit_cycles->setText(QString::number(cycles));

}

void MainWindow::SetBlockCTS(const SelectTests::BlockCTS &blockCTS)
{
    this->m_blockCTS = blockCTS;
    m_program->SetBlockCTS(blockCTS);
}

void MainWindow::SetSensorsNumber(quint8 num)
{
    num = 1;
    bool noSensors = (num == 0);

    std::array<bool, 6> tabState = {true, true, true, true, true, true};

    if (m_blockCTS.do_1 || m_blockCTS.do_2 || m_blockCTS.do_3 || m_blockCTS.do_4) {
        ui->groupBox_DO->setVisible(true);
    }

    if (m_blockCTS.moving) {
        // ui->groupBox_linear_motion_sensor->setVisible(true);

        ui->label_start_value->setVisible(true);
        ui->label_end_value->setVisible(true);

        ui->label_start_positio_sensor->setVisible(true);
        ui->label_end_positio_sensor->setVisible(true);
    } else {
        // ui->groupBox_linear_motion_sensor->setVisible(false);

        ui->label_start_value->setVisible(false);
        ui->label_end_value->setVisible(false);

        // ui->label_start_positio_sensor->setVisible(false);
        // ui->label_end_positio_sensor->setVisible(false);
    }

    std::vector<CTSRule> rules = {
        // 3) Комплексных; Запорно-Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
        {
            [](const SelectTests::BlockCTS& cts) {
                return cts.usb &&
                       cts.pressure_1 &&
                       cts.pressure_2 &&
                       cts.pressure_3 &&
                       cts.moving &&
                       cts.input_4_20_mA &&
                       cts.output_4_20_mA &&
                       cts.imit_switch_0_3 &&
                       cts.imit_switch_3_0 &&
                       (cts.do_1 || cts.do_2 || cts.do_3 || cts.do_4);
            },
            {true, true, true, true, true, true}
        },
        // 5) Комплексных; Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
        {
            [](const SelectTests::BlockCTS& cts) {
                return cts.usb &&
                       cts.pressure_1 &&
                       cts.pressure_2 &&
                       cts.pressure_3 &&
                       cts.moving &&
                       cts.input_4_20_mA &&
                       cts.output_4_20_mA;
            },
            {true, true, true, true, true, true}
        },
        // 2) Базовых; Запорно-Регулирующей Арматуры; Тесты: полного хода, циклический
        {
            [](const SelectTests::BlockCTS& cts) {
                return cts.usb &&
                       cts.input_4_20_mA &&
                       cts.output_4_20_mA &&
                       cts.imit_switch_0_3 &&
                       cts.imit_switch_3_0 &&
                       (cts.do_1 || cts.do_2 || cts.do_3 || cts.do_4);
            },
            {true, false, true, false, true, true}
        },
        // 1) Комплексных; Отсечной Арматуры; Тесты: полного хода, циклический
        {
            [](const SelectTests::BlockCTS& cts) {
                return cts.usb &&
                       cts.imit_switch_0_3 &&
                       cts.imit_switch_3_0 &&
                       (cts.do_1 || cts.do_2 || cts.do_3 || cts.do_4);
            },
            {true, false, true, false, true, true}
        },
        // 4) Базовых; Регулирующей Арматуры; Тесты: полного хода, циклический — все вкладки
        {
            [](const SelectTests::BlockCTS& cts) {
                return cts.usb &&
                       cts.input_4_20_mA &&
                       cts.output_4_20_mA;
            },
            {true, false, true, false, true, true}
        }
    };

    for (const auto& rule : rules) {
        if (rule.condition(m_blockCTS)) {
            tabState = rule.pattern;
            break;
        }
    }

    for (size_t i = 0; i < tabState.size(); ++i)
        ui->tabWidget->setTabEnabled(i, tabState[i]);

    ui->verticalSlider_task->setEnabled(!noSensors);
    ui->doubleSpinBox_task->setEnabled(!noSensors);
    ui->pushButton_stroke_start->setEnabled(!noSensors);
    ui->pushButton_tests_start->setEnabled(!noSensors);
    // ui->pushButton_main_start->setEnabled(num > 1);

    if (num > 0) {
        ui->checkBox_task->setVisible(num > 1);
        ui->checkBox_line->setVisible(num > 1);
        ui->checkBox_pressure1->setVisible(num > 1);
        ui->checkBox_pressure2->setVisible(num > 2);
        ui->checkBox_pressure3->setVisible(num > 3);

        ui->checkBox_task->setCheckState(num > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_line->setCheckState(num > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_pressure1->setCheckState(num > 1 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_pressure2->setCheckState(num > 2 ? Qt::Checked : Qt::Unchecked);
        ui->checkBox_pressure3->setCheckState(num > 3 ? Qt::Checked : Qt::Unchecked);
    }
}
void MainWindow::SetButtonInitEnabled(bool enable)
{
    ui->pushButton_init->setEnabled(enable);
}

void MainWindow::AddPoints(Charts chart, QVector<Point> points)
{
    for (const auto point : points)
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
        QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[Charts::Task]->getpoints(1);

        QPair<QList<QPointF>, QList<QPointF>> pointsPressure = m_charts[Charts::Pressure]->getpoints(0);

        points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
        points.push_back({pointsLinear.second.begin(), pointsLinear.second.end()});
        points.push_back({pointsPressure.first.begin(), pointsPressure.first.end()});
        points.push_back({pointsPressure.second.begin(), pointsPressure.second.end()});
    }
    if (chart == Charts::Step) {
        QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[Charts::Step]->getpoints(1);
        QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_charts[Charts::Step]->getpoints(0);

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
    if (m_testing) {
        if (QMessageBox::question(this, "Внимание!", "Вы действительно хотите завершить тест?")
            == QMessageBox::Yes) {
            emit StopTest();
        }
    } else {
        if (m_cyclicTestSettings->exec() != QDialog::Accepted)
            return;

        auto params = m_cyclicTestSettings->getParameters();
        qDebug() << "[CyclicTest] sequence=" << params.sequence
                 << " delay_sec=" << params.delay_sec
                 << " num_cycles=" << params.num_cycles;
        StartTest();
        emit StartCyclicSolenoidTest(params);
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
    ui->checkBox_DI1->setChecked((status & (1 << 0)) != 0);
    ui->checkBox_DI2->setChecked((status & (1 << 1)) != 0);
}

void MainWindow::InitCharts()
{
    ValveInfo *valveInfo = m_registry->GetValveInfo();
    bool rotate = (valveInfo->strokeMovement != 0);

    m_charts[Charts::Task] = ui->Chart_task;
    m_charts[Charts::Task]->setName("Task");
    m_charts[Charts::Task]->useTimeaxis(false);
    m_charts[Charts::Task]->addAxis("%.2f bar");

    if (!rotate)
        m_charts[Charts::Task]->addAxis("%.2f mm");
    else
        m_charts[Charts::Task]->addAxis("%.2f deg");

    m_charts[Charts::Task]->addSeries(1, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Task]->addSeries(1,"Датчик линейных перемещений",QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Task]->addSeries(0, "Датчик давления 1", QColor::fromRgb(0, 0, 255));
    m_charts[Charts::Task]->addSeries(0, "Датчик давления 2", QColor::fromRgb(0, 200, 0));
    m_charts[Charts::Task]->addSeries(0, "Датчик давления 3", QColor::fromRgb(150, 0, 200));


    m_charts[Charts::Friction] = ui->Chart_friction;
    m_charts[Charts::Friction]->setName("Friction");
    m_charts[Charts::Friction]->addAxis("%.2f H");
    m_charts[Charts::Friction]->addSeries(0, "Трение от перемещения", QColor::fromRgb(255, 0, 0));
    if (!rotate)
        m_charts[Charts::Friction]->setLabelXformat("%.2f mm");
    else
        m_charts[Charts::Friction]->setLabelXformat("%.2f deg");


    m_charts[Charts::Pressure] = ui->Chart_pressure;
    m_charts[Charts::Pressure]->setName("Pressure");
    m_charts[Charts::Pressure]->useTimeaxis(false);
    m_charts[Charts::Pressure]->setLabelXformat("%.2f bar");

    if (!rotate)
        m_charts[Charts::Pressure]->addAxis("%.2f mm");
    else
        m_charts[Charts::Pressure]->addAxis("%.2f deg");

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
    m_charts[Charts::Response]->addSeries(0,
                                          "Датчик линейных перемещений",
                                          QColor::fromRgb(255, 0, 0));

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
    m_charts[Charts::Trend]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Trend]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Trend]->setMaxRange(60000);

    m_charts[Charts::CyclicSolenoid] = ui->Chart_cyclic_solenoid;
    m_charts[Charts::CyclicSolenoid]->setName("Cyclic_solenoid");
    m_charts[Charts::CyclicSolenoid]->useTimeaxis(true); // new
    m_charts[Charts::CyclicSolenoid]->addAxis("%.2f%%");
    m_charts[Charts::CyclicSolenoid]->addSeries(0, "Задание", QColor::fromRgb(0, 0, 0));
    m_charts[Charts::CyclicSolenoid]->addSeries(0, "Датчик линейных перемещений", QColor::fromRgb(255, 0, 0));

    connect(m_program, &Program::AddPoints, this, &MainWindow::AddPoints);
    connect(m_program, &Program::ClearPoints, this, &MainWindow::ClearPoints);
    connect(m_program, &Program::DublSeries, this, &MainWindow::DublSeries);
    connect(m_program, &Program::SetVisible, this, &MainWindow::SetChartVisible);
    connect(m_program, &Program::SetRegressionEnable, this, &MainWindow::SetRegressionEnable);
    connect(m_program, &Program::ShowDots, this, &MainWindow::ShowDots);

    connect(ui->checkBox_task, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(0, k != 0);
    });

    connect(ui->checkBox_line, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(1, k != 0);
    });

    connect(ui->checkBox_pressure1, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(2, k != 0);
    });

    connect(ui->checkBox_pressure2, &QCheckBox::checkStateChanged, this, [&](int k) {
        m_charts[Charts::Task]->visible(3, k != 0);
    });

    connect(ui->checkBox_pressure3, &QCheckBox::checkStateChanged, this, [&](int k) {
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

    ui->checkBox_task->setCheckState(Qt::Unchecked);
    ui->checkBox_line->setCheckState(Qt::Unchecked);
    ui->checkBox_pressure1->setCheckState(Qt::Unchecked);
    ui->checkBox_pressure2->setCheckState(Qt::Unchecked);
    ui->checkBox_pressure3->setCheckState(Qt::Unchecked);

    ui->checkBox_task->setVisible(false);
    ui->checkBox_line->setVisible(false);
    ui->checkBox_pressure1->setVisible(false);
    ui->checkBox_pressure2->setVisible(false);
    ui->checkBox_pressure3->setVisible(false);
}

void MainWindow::SaveChart(Charts chart)
{
    m_reportSaver->SaveImage(m_charts[chart]);

    QPixmap pix = m_charts[chart]->grab();

    switch (chart) {
    case Charts::Task:
        ui->label_pixmap3->setPixmap(pix);
        break;
    case Charts::Stroke:
        break;
    case Charts::Response:
    case Charts::Resolution:
    case Charts::Step:
    case Charts::Pressure:
        ui->label_pixmap2->setPixmap(pix);
    case Charts::Friction:
        ui->label_pixmap1->setPixmap(pix);

    case Charts::Trend:
    case Charts::CyclicSolenoid:
        break;
    default:
        break;
    }

    ui->label_pixmap1->setScaledContents(true);
    ui->label_pixmap2->setScaledContents(true);
    ui->label_pixmap3->setScaledContents(true);
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

void MainWindow::InitReport()
{
    m_report.data.push_back({5, 4, ui->lineEdit_object});
    m_report.data.push_back({6, 4, ui->lineEdit_manufacture});
    m_report.data.push_back({7, 4, ui->lineEdit_department});

    m_report.data.push_back({5, 13, ui->lineEdit_positionNumber});
    m_report.data.push_back({6, 13, ui->lineEdit_serialNumber});
    m_report.data.push_back({7, 13, ui->lineEdit_valveModel});
    m_report.data.push_back({8, 13, ui->lineEdit_manufacturer});
    m_report.data.push_back({9, 13, ui->lineEdit_DNPN});
    m_report.data.push_back({10, 13, ui->lineEdit_positionerModel});
    m_report.data.push_back({11, 13, ui->lineEdit_pressure});
    m_report.data.push_back({12, 13, ui->lineEdit_safePosition});
    m_report.data.push_back({14, 13, ui->lineEdit_strokeMovement});

    m_report.data.push_back({26, 5, ui->lineEdit_dinamic_real});
    m_report.data.push_back({26, 8, ui->lineEdit_dinamic_recomend});
    m_report.data.push_back({28, 5, ui->lineEdit_dinamic_ip_real});
    m_report.data.push_back({28, 8, ui->lineEdit_dinamic_ip_recomend});
    m_report.data.push_back({30, 5, ui->lineEdit_stroke_real});
    m_report.data.push_back({30, 8, ui->lineEdit_stroke_recomend});
    m_report.data.push_back({32, 5, ui->lineEdit_range_real});
    m_report.data.push_back({32, 8, ui->lineEdit_range_recomend});
    m_report.data.push_back({34, 5, ui->lineEdit_range_pressure});
    m_report.data.push_back({36, 5, ui->lineEdit_friction_percent});
    m_report.data.push_back({38, 5, ui->lineEdit_friction});

    m_report.data.push_back({52, 5, ui->lineEdit_time_forward});
    m_report.data.push_back({52, 8, ui->lineEdit_time_backward});

    m_report.data.push_back({74, 4, ui->lineEdit_FIO});

    m_report.data.push_back({66, 12, ui->lineEdit_date});
    m_report.data.push_back({161, 12, ui->lineEdit_date});

    m_report.data.push_back({10, 5, ui->lineEdit_forwardSec});
    m_report.data.push_back({10, 5, ui->lineEdit_backwardSec});
    m_report.data.push_back({10, 5, ui->lineEdit_range_real});
    m_report.data.push_back({10, 5, ui->lineEdit_rangePercent});
    m_report.data.push_back({10, 5, ui->lineEdit_totalTimeSec});

    m_report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    m_report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    m_report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    m_report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    m_report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    m_report.validation.push_back({"=Заключение!$F$3", "E50"});
}
