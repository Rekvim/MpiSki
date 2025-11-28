#include "MainWindow.h"
#include "./Src/CustomChart/MyChart.h"
#include "ui_MainWindow.h"

#include "Src/ReportBuilders/ReportBuilder_B_CVT.h"
#include "Src/ReportBuilders/ReportBuilder_B_SACVT.h"
#include "Src/ReportBuilders/ReportBuilder_C_CVT.h"
#include "Src/ReportBuilders/ReportBuilder_C_SACVT.h"
#include "Src/ReportBuilders/ReportBuilder_C_SOVT.h"
#include "./Src/ValidatorFactory/ValidatorFactory.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    lockTabsForPreInit();

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

    m_lineEdits[TextObjects::LineEdit_linearSensor] = ui->lineEdit_linearSensor;
    m_lineEdits[TextObjects::LineEdit_linearSensorPercent] = ui->lineEdit_linearSensorPercent;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_1] = ui->lineEdit_pressureSensor_1;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_2] = ui->lineEdit_pressureSensor_2;
    m_lineEdits[TextObjects::LineEdit_pressureSensor_3] = ui->lineEdit_pressureSensor_3;
    m_lineEdits[TextObjects::LineEdit_feedback_4_20mA] = ui->lineEdit_feedback_4_20mA;

    m_program = new Program;
    m_programThread = new QThread(this);
    m_program->moveToThread(m_programThread);

    // соговое окно
    // logOutput = new QPlainTextEdit(this);
    // logOutput->setReadOnly(true);
    // logOutput->setStyleSheet("font-size: 8pt;");

    // auto *dock = new QDockWidget(tr("Лог"), this);
    // dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    // dock->setWidget(logOutput);

    // dock->setMinimumWidth(300);
    // // dock->resize(300, dock->height());

    // addDockWidget(Qt::RightDockWidgetArea, dock);

    // connect(m_program, &Program::errorOccured,
    //         this, &MainWindow::appendLog,
    //         Qt::QueuedConnection);

    // appendLog("Логовое окно инициализировано");

    m_durationTimer = new QTimer(this);
    m_durationTimer->setInterval(100);

    connect(m_durationTimer, &QTimer::timeout,
            this, &MainWindow::onCountdownTimeout);

    connect(m_program, &Program::totalTestTimeMs,
            this, &MainWindow::onTotalTestTimeMs);

    connect(this, &MainWindow::initialized,
            m_program, &Program::initialization);

    connect(this, &MainWindow::doInitStatesSelected,
            m_program, &Program::setInitDOStates);

    connect(ui->pushButton_set, &QPushButton::clicked,
            m_program, &Program::button_set_position);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            m_program, &Program::checkbox_autoInit);

    connect(this, &MainWindow::setDo,
            m_program, &Program::button_DO);

    for (int i = 0; i < 4; ++i) {
        auto btn = findChild<QPushButton*>(QString("pushButton_DO%1").arg(i));
        if (!btn) continue;

        connect(btn, &QPushButton::clicked,
                this, [this, i](bool checked)
                {
                    emit setDo(i, checked);
                });
    }

    connect(this, &MainWindow::runCyclicTest,
            m_program, &Program::startCyclicTest);

    connect(this, &MainWindow::runMainTest,
            m_program, &Program::startMainTest);

    connect(m_program, &Program::mainTestFinished,
            this, &MainWindow::promptSaveCharts);

    connect(this, &MainWindow::runStrokeTest,
            m_program, &Program::startStrokeTest);

    connect(this, &MainWindow::runOptionalTest,
            m_program, &Program::startOptionalTest);

    connect(this, &MainWindow::stopTest,
            m_program, &Program::terminateTest);

    connect(m_program, &Program::stopTheTest,
            this, &MainWindow::endTest);

    connect(m_program, &Program::setText,
            this, &MainWindow::setText);

    connect(m_program, &Program::setDoButtonsChecked,
            this, &MainWindow::setDoButtonsChecked);

    connect(m_program, &Program::setDiCheckboxesChecked,
            this, &MainWindow::setDiCheckboxesChecked);

    connect(this, &MainWindow::dacValueRequested,
            m_program, &Program::setDAC_real);

    connect(ui->doubleSpinBox_task,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,[&](double value) {
                if (qRound(value * 1000) != ui->verticalSlider_task->value()) {
                    if (ui->verticalSlider_task->isEnabled())
                        emit dacValueRequested(value);
                    ui->verticalSlider_task->setValue(qRound(value * 1000));
                }
            });

    connect(ui->verticalSlider_task, &QSlider::valueChanged,
            this, [&](int value) {
                if (qRound(ui->doubleSpinBox_task->value() * 1000) != value) {
                    if (ui->doubleSpinBox_task->isEnabled())
                        emit dacValueRequested(value / 1000.0);
                    ui->doubleSpinBox_task->setValue(value / 1000.0);
                }
            });

    connect(this, &MainWindow::patternChanged,
            m_program, &Program::setPattern);

    connect(m_program, &Program::setTask,
            this, &MainWindow::setTask);

    connect(m_program, &Program::setSensorNumber,
            this, &MainWindow::setSensorsNumber);

    connect(m_program, &Program::setButtonInitEnabled,
            this, &MainWindow::setButtonInitEnabled);

    connect(m_program, &Program::setTaskControlsEnabled,
            this, &MainWindow::setTaskControlsEnabled);

    connect(m_program, &Program::setStepResults,
            this, &MainWindow::setStepTestResults);

    connect(m_program, &Program::getParameters_mainTest,
            this, &MainWindow::receivedParameters_mainTest,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_stepTest,
            this, &MainWindow::receivedParameters_stepTest,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_resolutionTest,
            this, &MainWindow::receivedParameters_resolutionTest,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_responseTest,
            this, &MainWindow::receivedParameters_responseTest,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getParameters_cyclicTest,
            this, &MainWindow::receivedParameters_cyclicTest,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::question,
            this, &MainWindow::question,
            Qt::BlockingQueuedConnection);

    connect(m_reportSaver, &ReportSaver::Question,
            this, &MainWindow::question,
            Qt::DirectConnection);

    connect(m_reportSaver, &ReportSaver::GetDirectory,
            this, &MainWindow::getDirectory,
            Qt::DirectConnection);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                ui->pushButton_set->setEnabled(!state);
            });

    ui->tableWidget_stepResults->setColumnCount(2);
    ui->tableWidget_stepResults->setHorizontalHeaderLabels({QLatin1String("T86"), tr("Перерегулирование")});
    ui->tableWidget_stepResults->resizeColumnsToContents();

    ui->label_arrowUp->setCursor(Qt::PointingHandCursor);
    ui->label_arrowDown->setCursor(Qt::PointingHandCursor);

    ui->label_arrowUp->installEventFilter(this);
    ui->label_arrowDown->installEventFilter(this);

    connect(m_program, &Program::telemetryUpdated,
            this, &MainWindow::onTelemetryUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Program::testFinished,
            this, &MainWindow::endTest);

    connect(m_program, &Program::cyclicCycleCompleted,
            this, [this](int completed){
                int remaining = completed;
                ui->label_cyclicTest_completedCyclesValue->setText(QString::number(remaining));
            });
}

MainWindow::~MainWindow()
{
    QMetaObject::invokeMethod(
        m_program, "terminateTest",
        Qt::BlockingQueuedConnection);

    m_programThread->quit();
    m_programThread->wait();

    delete ui;
}

void MainWindow::lockTabsForPreInit()
{
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tab_mainTests), false);
    ui->tabWidget->setTabEnabled(1, false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    ui->tabWidget->setTabEnabled(4, false);
}

void MainWindow::updateAvailableTabs()
{
    displayDependingPattern();
    if (!m_isInitialized) {
        lockTabsForPreInit();
        return;
    }
}

void MainWindow::onCountdownTimeout()
{
    const qint64 elapsed = m_elapsedTimer.elapsed();
    qint64 remaining = static_cast<qint64>(m_totalTestMs) - elapsed;
    if (remaining < 0) remaining = 0;

    const auto formatHMS = [](quint64 ms) -> QString {
        quint64 h = ms / 3600000ULL;  ms %= 3600000ULL;
        quint64 m = ms / 60000ULL;    ms %= 60000ULL;
        quint64 s = ms / 1000ULL;
        quint64 x = ms % 1000ULL;
        return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(x, 3, 10, QChar('0'));
    };

    ui->statusbar->showMessage(
        tr("Тест в процессе. До завершения теста осталось: %1 (прошло %2 из %3)")
            .arg(formatHMS(static_cast<quint64>(remaining)))
            .arg(formatHMS(static_cast<quint64>(elapsed)))
            .arg(formatHMS(m_totalTestMs))
        );

    if (remaining == 0) m_durationTimer->stop();
}

void MainWindow::onTotalTestTimeMs(quint64 totalMs)
{
    m_totalTestMs = totalMs;
    m_elapsedTimer.restart();

    if (!m_durationTimer) {
        m_durationTimer = new QTimer(this);
        m_durationTimer->setInterval(250);
        connect(m_durationTimer, &QTimer::timeout,
                this, &MainWindow::onCountdownTimeout);
    }

    auto formatHMS = [](quint64 ms) -> QString {
        const quint64 hours = ms / 3600000ULL;
        ms %= 3600000ULL;
        const quint64 minutes = ms / 60000ULL;
        ms %= 60000ULL;
        const quint64 seconds = ms / 1000ULL;
        const quint64 msec = ms % 1000ULL;

        return QString("%1:%2:%3.%4")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
            .arg(msec, 3, 10, QChar('0'));
    };

    ui->statusbar->showMessage(
        tr("Плановая длительность теста: %1").arg(formatHMS(m_totalTestMs))
        );

    m_durationTimer->start();
    onCountdownTimeout();
}

static void setIndicatorColor(QWidget* widget, const QString& color, const QString& border) {
    widget->setStyleSheet(QString(
                              "background: %1;"
                              "border: 4px solid %2;"
                              "border-radius: 10px;"
                              ).arg(color, border));
}

static void updateIndicator(double value,
                            double lowerLimit,
                            double upperLimit,
                            QWidget* widget)
{
    if (lowerLimit > upperLimit)
        std::swap(lowerLimit, upperLimit);

    if (value < lowerLimit || value > upperLimit) {
        setIndicatorColor(widget,
                          QLatin1String("#B80F0F"),
                          QLatin1String("#510000"));
    }

    else {
        setIndicatorColor(widget,
                          QLatin1String("#4E8448"),
                          QLatin1String("#16362B"));
    }
}

void MainWindow::updateLinearLimitStatus()
{
    bool okValue = false;
    double value = ui->lineEdit_crossingLimits_linearCharacteristic_value
                       ->text().toDouble(&okValue);
    if (!okValue) return;

    bool okLow = false, okHigh = false;
    double lower = ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit
                       ->text().toDouble(&okLow);
    double upper = ui->lineEdit_crossingLimits_linearCharacteristic_upperLimit
                       ->text().toDouble(&okHigh);

    if (!okLow || !okHigh) return;

    updateIndicator(value,
                    lower,
                    upper,
                    ui->widget_crossingLimits_linearCharacteristic_limitStatusIndicator);
}

void MainWindow::updateFrictionForceLimitStatus()
{
    bool okValue = false;
    double value = ui->lineEdit_crossingLimits_coefficientFriction_value
                       ->text().toDouble(&okValue);
    if (!okValue) return;

    bool okLow = false, okHigh = false;
    double lower = ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit
                       ->text().toDouble(&okLow);
    double upper = ui->lineEdit_crossingLimits_coefficientFriction_upperLimit
                       ->text().toDouble(&okHigh);

    if (!okLow || !okHigh) return;

    updateIndicator(value,
                    lower,
                    upper,
                    ui->widget_crossingLimits_coefficientFriction_limitStatusIndicator);
}

static void updateRangeIndicator(double valueLow, double valueHigh, double limitLow, double limitHigh, QWidget* widget) {
    if (limitLow > limitHigh) std::swap(limitLow, limitHigh);
    if (valueLow < limitLow || valueHigh > limitHigh) {
        setIndicatorColor(widget,
                          QLatin1String("#B80F0F"),
                          QLatin1String("#510000"));
    }
    else { setIndicatorColor(widget, QLatin1String("#4E8448"), QLatin1String("#16362B")); } }

void MainWindow::updateSpringLimitStatus()
{
    if (!ui->widget_crossingLimits_spring->isVisible())
        return;

    QString text = ui->lineEdit_crossingLimits_spring_value->text();
    if (text.isEmpty())
        return;

    const QRegularExpression re(R"(\s*[-–]\s*)");
    const QStringList parts = text.split(re);
    if (parts.size() != 2)
        return;

    bool okLow = false;
    bool okHigh = false;
    double valueLow  = parts.at(0).toDouble(&okLow);
    double valueHigh = parts.at(1).toDouble(&okHigh);
    if (!okLow || !okHigh)
        return;

    double limitLow  = ui->lineEdit_crossingLimits_spring_lowerLimit->text().toDouble();
    double limitHigh = ui->lineEdit_crossingLimits_spring_upperLimit->text().toDouble();

    updateRangeIndicator(valueLow, valueHigh,
                         limitLow, limitHigh,
                         ui->widget_crossingLimits_spring_limitStatusIndicator);
}

void MainWindow::updateRangeLimitStatus()
{
    bool okValue = false;
    double value = ui->lineEdit_crossingLimits_range_value
                       ->text().toDouble(&okValue);
    if (!okValue) return;

    bool okLow = false, okHigh = false;
    double lower = ui->lineEdit_crossingLimits_range_lowerLimit
                       ->text().toDouble(&okLow);
    double upper = ui->lineEdit_crossingLimits_range_upperLimit
                       ->text().toDouble(&okHigh);
    if (!okLow || !okHigh) return;

    updateIndicator(value,
                    lower,
                    upper,
                    ui->widget_crossingLimits_range_limitStatusIndicator);
}
void MainWindow::updateDynamicErrorLimitStatus()
{
    bool okValue = false;
    double value = ui->lineEdit_crossingLimits_dynamicError_value
                       ->text().toDouble(&okValue);
    if (!okValue) return;

    bool okLow = false, okHigh = false;
    double lower = ui->lineEdit_crossingLimits_dynamicError_lowerLimit
                       ->text().toDouble(&okLow);
    double upper = ui->lineEdit_crossingLimits_dynamicError_upperLimit
                       ->text().toDouble(&okHigh);
    if (!okLow || !okHigh) return;

    updateIndicator(value,
                    lower,
                    upper,
                    ui->widget_crossingLimits_dynamicError_limitStatusIndicator);
}


void MainWindow::onTelemetryUpdated(const TelemetryStore &telemetry) {

    m_telemetryStore = telemetry;

    ui->label_deviceStatusValue->setText(telemetry.init.deviceStatusText);
    ui->label_deviceStatusValue->setStyleSheet(
        "color:" + telemetry.init.deviceStatusColor.name(QColor::HexRgb));

    ui->label_deviceInitValue->setText(telemetry.init.initStatusText);
    ui->label_deviceInitValue->setStyleSheet(
        "color:" + telemetry.init.initStatusColor.name(QColor::HexRgb));

    ui->label_connectedSensorsNumber->setText(telemetry.init.connectedSensorsText);
    ui->label_connectedSensorsNumber->setStyleSheet(
        "color:" + telemetry.init.connectedSensorsColor.name(QColor::HexRgb));

    ui->label_startingPositionValue->setText(telemetry.init.startingPositionText);
    ui->label_startingPositionValue->setStyleSheet(
        "color:" + telemetry.init.startingPositionColor.name(QColor::HexRgb));

    ui->label_finalPositionValue->setText(telemetry.init.finalPositionText);
    ui->label_finalPositionValue->setStyleSheet(
        "color:" + telemetry.init.finalPositionColor.name(QColor::HexRgb));

    ui->label_pressureDifferenceValue->setText(
        QString("%1 bar")
            .arg(telemetry.mainTestRecord.pressureDifference, 0, 'f', 3)
        );
    ui->label_frictionForceValue->setText(
        QString("%1 H")
            .arg(telemetry.mainTestRecord.frictionForce, 0, 'f', 3)
        );
    ui->label_frictionPercentValue->setText(
        QString("%1 %")
            .arg(telemetry.mainTestRecord.frictionPercent, 0, 'f', 2)
        );
    ui->lineEdit_frictionForceValue->setText(
        QString("%1")
            .arg(telemetry.mainTestRecord.frictionForce, 0, 'f', 3)
        );
    ui->lineEdit_frictionPercentValue->setText(
        QString("%1")
            .arg(telemetry.mainTestRecord.frictionPercent, 0, 'f', 2)
        );

    ui->label_dynamicErrorMeanPercent->setText(
        QString("%1 %")
            .arg(telemetry.mainTestRecord.dynamicError_meanPercent, 0, 'f', 2)
        );
    ui->label_dynamicErrorMean->setText(
        QString("%1 mA")
            .arg(telemetry.mainTestRecord.dynamicError_mean, 0, 'f', 3)
        );
    ui->label_dynamicErrorMaxPercent->setText(
        QString("%1 %")
            .arg(telemetry.mainTestRecord.dynamicError_maxPercent, 0, 'f', 2)
        );

    ui->label_dynamicErrorMax->setText(
        QString("%1 mA")
            .arg(telemetry.mainTestRecord.dynamicError_max, 0, 'f', 3)
        );
    ui->lineEdit_dynamicErrorReal->setText(
        QString("%1")
            .arg(telemetry.mainTestRecord.dynamicErrorReal, 0, 'f', 2)
        );

    ui->label_dynamicErrorMax->setText(
        QString("%1 bar")
            .arg(telemetry.mainTestRecord.lowLimitPressure, 0, 'f', 2)
        );
    ui->label_dynamicErrorMax->setText(
        QString("%1 bar")
            .arg(telemetry.mainTestRecord.highLimitPressure, 0, 'f', 2)
        );

    ui->label_valveStroke_range->setText(
        QString("%1")
            .arg(telemetry.valveStrokeRecord.range)
        );

    // StrokeRecord
    ui->lineEdit_strokeReal->setText(
        QString("%1").arg(telemetry.valveStrokeRecord.real, 0, 'f', 2));



    ui->label_lowLimitValue->setText(
        QString("%1")
            .arg(telemetry.mainTestRecord.lowLimitPressure)
        );
    ui->label_highLimitValue->setText(
        QString("%1")
            .arg(telemetry.mainTestRecord.highLimitPressure)
        );

    ui->lineEdit_rangePressure->setText(
        QString("%1–%2")
            .arg(telemetry.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetry.mainTestRecord.highLimitPressure, 0, 'f', 2)
        );

    ui->lineEdit_driveRangeReal->setText(
        QString("%1–%2")
            .arg(telemetry.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetry.mainTestRecord.springHigh, 0, 'f', 2)
        );

    // StrokeTest

    ui->label_strokeTest_forwardTime->setText(telemetry.strokeTestRecord.timeForwardMs);
    ui->lineEdit_strokeTest_forwardTime->setText(telemetry.strokeTestRecord.timeForwardMs);

    ui->label_strokeTest_backwardTime->setText(telemetry.strokeTestRecord.timeBackwardMs);
    ui->lineEdit_strokeTest_backwardTime->setText(telemetry.strokeTestRecord.timeBackwardMs);

    // CyclicTestResults
    // ui->label_cyclicTest_sequenceValue->setText(TS.cyclicTestRecord.sequence);
    // ui->label_cyclicTest_specifiedCyclesValue->setText(
    //     QString::number(TS.cyclicTestRecord.cycles));

    // qint64 millis = qint64(TS.cyclicTestRecord.totalTimeSec * 1000.0);
    // QTime tC(0, 0);
    // tC = tC.addMSecs(millis);
    // ui->label_cyclicTest_totalTimeValue->setText(
    //     tC.toString("hh:mm:ss.zzz"));

    // crossing

    // 1) Динамическая ошибка: одно значение
    ui->lineEdit_crossingLimits_dynamicError_value->setText(
        QString::number(telemetry.mainTestRecord.dynamicErrorReal, 'f', 2));

    // 2) Линейность характеристики
    ui->lineEdit_crossingLimits_linearCharacteristic_value->setText(
        QString::number(telemetry.mainTestRecord.linearityError, 'f', 2));

    // 3) Ход клапана: одно значение (реальный ход)
    ui->lineEdit_crossingLimits_range_value->setText(
        QString::number(telemetry.valveStrokeRecord.real, 'f', 2));

    // 4) Диапазон пружины: low–high
    ui->lineEdit_crossingLimits_spring_value->setText(
        QString("%1–%2")
            .arg(telemetry.mainTestRecord.springLow,  0, 'f', 2)
            .arg(telemetry.mainTestRecord.springHigh, 0, 'f', 2));

    // 5) Коэффициент / процент трения: ОДНО значение,
    ui->lineEdit_crossingLimits_coefficientFriction_value->setText(
        QString::number(telemetry.mainTestRecord.frictionPercent, 'f', 2));

    // Обновляем индикаторы
    updateFrictionForceLimitStatus();
    updateSpringLimitStatus();
    updateRangeLimitStatus();
    updateDynamicErrorLimitStatus();
}

void MainWindow::appendLog(const QString& text) {
    const QString stamp = QDateTime::currentDateTime()
    .toString("[hh:mm:ss.zzz] ");
    m_logOutput->appendPlainText(stamp + text);
}

void MainWindow::on_pushButton_signal_4mA_clicked()
{
    ui->doubleSpinBox_task->setValue(4.0);
}
void MainWindow::on_pushButton_signal_8mA_clicked()
{
    ui->doubleSpinBox_task->setValue(8.0);
}
void MainWindow::on_pushButton_signal_12mA_clicked()
{
    ui->doubleSpinBox_task->setValue(12.0);
}
void MainWindow::on_pushButton_signal_16mA_clicked()
{
    ui->doubleSpinBox_task->setValue(16.0);
}
void MainWindow::on_pushButton_signal_20mA_clicked()
{
    ui->doubleSpinBox_task->setValue(20.0);
}

void MainWindow::setTaskControlsEnabled(bool enable)
{
    ui->verticalSlider_task->setEnabled(enable);
    ui->doubleSpinBox_task->setEnabled(enable);
    ui->groupBox_SettingCurrentSignal->setEnabled(enable);
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

void MainWindow::setRegistry(Registry *registry)
{
    m_registry = registry;

    ObjectInfo *objectInfo = m_registry->getObjectInfo();
    ValveInfo *valveInfo = m_registry->getValveInfo();
    OtherParameters *otherParameters = m_registry->getOtherParameters();
    const CrossingLimits &limits = valveInfo->crossingLimits;

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

    const bool anyCrossingEnabled =
        limits.frictionEnabled
        || limits.linearCharacteristicEnabled
        || limits.rangeEnabled
        || limits.springEnabled
        || limits.dynamicErrorEnabled;

    // Если ничего не выбрано — прячем общий блок и выходим
    ui->widget_crossingLimits->setVisible(anyCrossingEnabled);

    ui->lineEdit_strokeRecomend->setText(valveInfo->strokValve);
    ui->lineEdit_driveRangeRecomend->setText(valveInfo->driveRecomendRange);

    ui->widget_crossingLimits_frictionForce->setVisible(limits.frictionEnabled);
    ui->widget_crossingLimits_linearCharacteristic->setVisible(limits.linearCharacteristicEnabled);
    ui->widget_crossingLimits_range->setVisible(limits.rangeEnabled);
    ui->widget_crossingLimits_spring->setVisible(limits.springEnabled);
    ui->widget_crossingLimits_dynamicError->setVisible(limits.dynamicErrorEnabled);

    if (limits.frictionEnabled) {
        ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->setText(
            QString::number(limits.frictionCoefLowerLimit, 'f', 2));
        ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->setText(
            QString::number(limits.frictionCoefUpperLimit, 'f', 2));
    }

    if (limits.linearCharacteristicEnabled) {
        ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_linearCharacteristic_upperLimit->setText(
            QString::number(limits.linearCharacteristicLowerLimit, 'f', 2));
    }

    if (limits.rangeEnabled) {
        ui->lineEdit_crossingLimits_range_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_range_upperLimit->setText(
            QString::number(limits.rangeUpperLimit, 'f', 2));
    }

    if (limits.springEnabled) {
        ui->lineEdit_crossingLimits_spring_lowerLimit->setText(
            QString::number(limits.springLowerLimit, 'f', 2));
        ui->lineEdit_crossingLimits_spring_upperLimit->setText(
            QString::number(limits.springUpperLimit, 'f', 2));
    }

    if (limits.dynamicErrorEnabled) {
        ui->lineEdit_crossingLimits_dynamicError_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_dynamicError_upperLimit->setText(QString::number(valveInfo->dinamicErrorRecomend, 'f', 2));
    }

    ui->lineEdit_strokeRecomend->setText(valveInfo->strokValve);
    ui->lineEdit_driveRangeRecomend->setText(valveInfo->driveRecomendRange);

    if (valveInfo->safePosition != 0) {
        m_stepTestSettings->reverse();
        m_responseTestSettings->reverse();
        m_resolutionTestSettings->reverse();
    }

    initCharts();

    m_program->setRegistry(registry);
    m_programThread->start();

    m_reportSaver->setRegistry(registry);
}

void MainWindow::setText(TextObjects object, const QString &text)
{
    if (m_lineEdits.contains(object)) {
        m_lineEdits[object]->setText(text);
    }
}

void MainWindow::setTask(qreal task)
{
    quint16 i_task = qRound(task * 1000);

    if (ui->doubleSpinBox_task->value() != i_task / 1000.0) {
        ui->doubleSpinBox_task->setValue(i_task / 1000.0);
    }

    if (ui->verticalSlider_task->value() != i_task) {
        ui->verticalSlider_task->setSliderPosition(i_task);
    }
}

void MainWindow::setStepTestResults(const QVector<StepTest::TestResult> &results, quint32 T_value)
{
    ui->tableWidget_stepResults->setHorizontalHeaderLabels(
        {tr(("T%1")).arg(T_value), tr("Перерегулирование")});

    ui->tableWidget_stepResults->setRowCount(results.size());
    QStringList rowNames;
    for (int i = 0; i < results.size(); ++i) {

        QString time = results.at(i).T_value == 0
                           ? tr("Ошибка")
                           : QTime(0, 0).addMSecs(results.at(i).T_value).toString("m:ss.zzz");

        ui->tableWidget_stepResults->setItem(i, 0, new QTableWidgetItem(time));

        QString overshoot = QString("%1%").arg(results.at(i).overshoot, 4, 'f', 2);
        ui->tableWidget_stepResults->setItem(i, 1, new QTableWidgetItem(overshoot));

        QString rowName = QString("%1-%2").arg(results.at(i).from)
                              .arg(results.at(i).to);
        rowNames << rowName;
    }
    ui->tableWidget_stepResults->setVerticalHeaderLabels(rowNames);
    ui->tableWidget_stepResults->resizeColumnsToContents();
}

void MainWindow::displayDependingPattern() {
    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT:
        ui->groupBox_DO->setVisible(false);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_B_SACVT:
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_CVT:
        ui->groupBox_DO->setVisible(false);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, true);
        ui->tabWidget->setTabEnabled(3, true);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_SACVT:
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, true);
        ui->tabWidget->setTabEnabled(3, true);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_SOVT:
        ui->groupBox_SettingCurrentSignal->setVisible(false);
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, true);
        break;
    default:
        break;
    }
}

void MainWindow::setSensorsNumber(quint8 num)
{
    bool hasSensors = (num == 0);

    if (num > 0) {
        m_isInitialized = true;
    }
    updateAvailableTabs();

    if (!m_blockCTS.moving) {
        ui->label_startingPositionValue->setVisible(false);
        ui->label_startingPosition->setVisible(false);
    }

    ui->groupBox_SettingCurrentSignal->setEnabled(!hasSensors);

    ui->pushButton_mainTest_start->setEnabled(num > 1);
    ui->pushButton_strokeTest_start->setEnabled(!hasSensors);
    ui->pushButton_optionalTests_start->setEnabled(!hasSensors);
    ui->pushButton_cyclicTest_start->setEnabled(!hasSensors);

    ui->doubleSpinBox_task->setEnabled(!hasSensors);
    ui->verticalSlider_task->setEnabled(!hasSensors);

    displayDependingPattern();

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
void MainWindow::setButtonInitEnabled(bool enable)
{
    ui->pushButton_init->setEnabled(enable);
}

void MainWindow::addPoints(Charts chart, const QVector<Point> &points)
{
    for (const auto& point : points)
        m_charts[chart]->addPoint(point.series_num, point.X, point.Y);
}

void MainWindow::clearPoints(Charts chart)
{
    m_charts[chart]->clear();
}

void MainWindow::setChartVisible(Charts chart, quint16 series, bool visible)
{
    m_charts[chart]->visible(series, visible);
}

void MainWindow::showDots(bool visible)
{
    m_charts[Charts::Task]->showdots(visible);
    m_charts[Charts::Pressure]->showdots(visible);
}

void MainWindow::duplicateMainChartsSeries()
{
    m_charts[Charts::Task]->duplicateChartSeries(1);
    m_charts[Charts::Task]->duplicateChartSeries(2);
    m_charts[Charts::Task]->duplicateChartSeries(3);
    m_charts[Charts::Task]->duplicateChartSeries(4);
    m_charts[Charts::Pressure]->duplicateChartSeries(0);
}

void MainWindow::onMainTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[chart]->getPoints(1);
    QPair<QList<QPointF>, QList<QPointF>> pointsPressure = m_charts[Charts::Pressure]->getPoints(0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsLinear.second.begin(), pointsLinear.second.end()});
    points.push_back({pointsPressure.first.begin(), pointsPressure.first.end()});
    points.push_back({pointsPressure.second.begin(), pointsPressure.second.end()});
}
void MainWindow::onStepTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    if (chart == Charts::Step) {
        QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[chart]->getPoints(1);
        QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_charts[chart]->getPoints(0);

        points.clear();
        points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
        points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
    }
}
void MainWindow::onCyclicTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        QPair<QList<QPointF>, QList<QPointF>> opened = m_charts[chart]->getPoints(3);
        QPair<QList<QPointF>, QList<QPointF>> closed = m_charts[chart]->getPoints(2);

        points.push_back({opened.first.begin(), opened.first.end()});
        points.push_back({closed.first.begin(), closed.first.end()});
    }

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_charts[chart]->getPoints(1);
    QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_charts[chart]->getPoints(0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
}

void MainWindow::setRegressionEnabled(bool enabled)
{
    ui->checkBox_regression->setEnabled(enabled);
    ui->checkBox_regression->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::receivedParameters_mainTest(MainTestSettings::TestParameters &parameters)
{
    if (m_mainTestSettings->exec() == QDialog::Accepted) {
        parameters = m_mainTestSettings->getParameters();
    } else {
        parameters.delay = 0;
    }
}

void MainWindow::receivedParameters_stepTest(StepTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_stepTestSettings->exec() == QDialog::Accepted) {
        parameters = m_stepTestSettings->getParameters();
        return;
    } else {
        parameters = {};
        return;
    }
}

void MainWindow::receivedParameters_resolutionTest(OtherTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_resolutionTestSettings->exec() == QDialog::Accepted) {
        parameters = m_resolutionTestSettings->getParameters();
        return;
    } else {
        parameters = {};
        return;
    }
}

void MainWindow::receivedParameters_responseTest(OtherTestSettings::TestParameters &parameters)
{
    parameters.points.clear();

    if (m_responseTestSettings->exec() == QDialog::Accepted) {
        parameters = m_responseTestSettings->getParameters();
        return;

    } else {
        parameters = {};
        return;
    }
}


static QString seqToString(const QVector<quint16>& seq)
{
    QStringList parts;
    parts.reserve(seq.size());
    for (quint16 v : seq) parts << QString::number(v);
    return parts.join('-');
}

void MainWindow::receivedParameters_cyclicTest(CyclicTestSettings::TestParameters &parameters)
{
    if (m_cyclicTestSettings->exec() == QDialog::Accepted) {
        using TP = CyclicTestSettings::TestParameters;
        parameters = m_cyclicTestSettings->getParameters();

        switch (parameters.testType) {
        case TP::Regulatory:
            ui->label_cyclicTest_sequenceValue->setText(seqToString(parameters.regSeqValues));
            ui->label_cyclicTest_specifiedCyclesValue->setText(
                QString::number(parameters.regulatory_numCycles));
            break;
        case TP::Shutoff:
            ui->label_cyclicTest_sequenceValue->setText(seqToString(parameters.offSeqValues));
            ui->label_cyclicTest_specifiedCyclesValue->setText(
                QString::number(parameters.shutoff_numCycles));
            break;
        default:
            ui->label_cyclicTest_sequenceValue->clear();
            ui->label_cyclicTest_specifiedCyclesValue->clear();
            break;
        }

        if (parameters.testType == TP::Regulatory && parameters.regulatory_enable_20mA) {
            emit dacValueRequested(20.0);
        }

        qint64 totalMs = 0;
        switch (parameters.testType) {
        case TP::Regulatory: {
            const auto raw = parameters.regSeqValues;
            quint64 steps = static_cast<quint64>(raw.size()) * parameters.regulatory_numCycles;

            totalMs = steps * (parameters.regulatory_delayMs
                               + parameters.regulatory_holdMs);
            break;
        }
        case TP::Shutoff: {
            const auto raw = parameters.offSeqValues;
            quint64 steps = static_cast<quint64>(raw.size()) * parameters.shutoff_numCycles;
            totalMs = steps * (parameters.shutoff_holdMs
                               + parameters.shutoff_delayMs);

            break;
        }
        case TP::Combined: {
            const auto regRaw = parameters.regSeqValues;
            quint64 regSteps = static_cast<quint64>(regRaw.size()) * parameters.regulatory_numCycles;
            quint64 regMs = regSteps * (parameters.regulatory_delayMs
                                        + parameters.regulatory_holdMs)
                            + parameters.regulatory_delayMs;

            const auto offRaw = parameters.offSeqValues;
            quint64 offSteps = static_cast<quint64>(offRaw.size()) * parameters.shutoff_numCycles;
            quint64 offMs = offSteps * (parameters.shutoff_holdMs
                                        + parameters.shutoff_delayMs);

            totalMs = regMs + offMs;
            break;
        }
        default:
            break;
        }

        QTime t0(0, 0);
        t0 = t0.addMSecs(totalMs);
        ui->label_cyclicTest_totalTimeValue->setText(t0.toString("hh:mm:ss.zzz"));

        return;
    } else {
        return;
    }
}

void MainWindow::question(const QString &title, const QString &text, bool &result)
{
    result = (QMessageBox::question(NULL, title, text) == QMessageBox::Yes);
}

void MainWindow::getDirectory(const QString &currentPath, QString &result)
{
    result = QFileDialog::getExistingDirectory(this,
                                               tr("Выберите папку для сохранения изображений"),
                                               currentPath);
}

void MainWindow::startTest()
{
    m_isTestRunning  = true;
    ui->statusbar->showMessage(tr("Тест в процессе"));
}

void MainWindow::endTest()
{
    m_isTestRunning  = false;

    ui->statusbar->showMessage(tr("Тест завершён"));

    if (m_durationTimer) {
        m_durationTimer->stop();
    }

    if (m_isUserCanceled ) {
        ui->label_cyclicTest_totalTimeValue->clear();
        ui->label_cyclicTest_specifiedCyclesValue->clear();
        ui->label_cyclicTest_sequenceValue->clear();
        ui->label_cyclicTest_completedCyclesValue->clear();
    }
}

void MainWindow::on_pushButton_mainTest_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
        == QMessageBox::Yes) {
            m_isUserCanceled  = true;
            emit stopTest();
        }
    } else {
        m_isUserCanceled  = false;
        emit runMainTest();
        startTest();
    }
}
void MainWindow::on_pushButton_mainTest_save_clicked()
{
    if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_task) {
        saveChart(Charts::Task);
    } else if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_pressure) {
        saveChart(Charts::Pressure);
    } else if (ui->tabWidget_mainTests->currentWidget() == ui->tab_mainTests_friction) {
        saveChart(Charts::Friction);
    }
}

void MainWindow::promptSaveCharts()
{
    if (m_isUserCanceled )
        return;

    auto answer = QMessageBox::question(
        this,
        tr("Сохранение результатов"),
        tr("Тест MainTest завершён.\nСохранитаь графики Task, Pressure и Friction?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (answer == QMessageBox::Yes) {
        saveChart(Charts::Task);
        saveChart(Charts::Pressure);
        saveChart(Charts::Friction);
    }
}

void MainWindow::on_pushButton_strokeTest_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
            == QMessageBox::Yes) {
            emit stopTest();
        }
    } else {
        emit runStrokeTest();
        startTest();
    }
}
void MainWindow::on_pushButton_strokeTest_save_clicked()
{
    saveChart(Charts::Stroke);
}

void MainWindow::on_pushButton_optionalTests_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
            == QMessageBox::Yes) {
            emit stopTest();
        }
    } else {
        emit runOptionalTest(ui->tabWidget_optionalTests->currentIndex());
        startTest();
    }
}
void MainWindow::on_pushButton_optionalTests_save_clicked()
{
    if (ui->tabWidget_optionalTests->currentWidget() == ui->tab_optionalTests_response) {
        saveChart(Charts::Response);
    } else if (ui->tabWidget_optionalTests->currentWidget() == ui->tab_optionalTests_resolution) {
        saveChart(Charts::Resolution);
    } else if (ui->tabWidget_optionalTests->currentWidget() == ui->tab_optionalTests_step) {
        saveChart(Charts::Step);
    }
}

void MainWindow::on_pushButton_cyclicTest_start_clicked()
{
    if (m_isTestRunning ) {
        if (QMessageBox::question(this, tr("Внимание!"), tr("Вы действительно хотите завершить тест?"))
            == QMessageBox::Yes) {
            m_isUserCanceled  = true;
            emit stopTest();
        }
        return;
    }

    m_isUserCanceled  = false;

    m_cyclicTestSettings->setPattern(m_patternType);

    emit runCyclicTest();
    startTest();
}

void MainWindow::on_pushButton_cyclicTest_save_clicked()
{
    saveChart(Charts::Cyclic);
}

void MainWindow::setDoButtonsChecked(quint8 bitmask)
{
    ui->pushButton_DO0->blockSignals(true);
    ui->pushButton_DO1->blockSignals(true);
    ui->pushButton_DO2->blockSignals(true);
    ui->pushButton_DO3->blockSignals(true);

    ui->pushButton_DO0->setChecked((bitmask & (1 << 0)) != 0);
    ui->pushButton_DO1->setChecked((bitmask & (1 << 1)) != 0);
    ui->pushButton_DO2->setChecked((bitmask & (1 << 2)) != 0);
    ui->pushButton_DO3->setChecked((bitmask & (1 << 3)) != 0);

    ui->pushButton_DO0->blockSignals(false);
    ui->pushButton_DO1->blockSignals(false);
    ui->pushButton_DO2->blockSignals(false);
    ui->pushButton_DO3->blockSignals(false);

    ui->groupBox_DO->setEnabled(true);
}

void MainWindow::setDiCheckboxesChecked(quint8 bitmask)
{
    ui->checkBox_switch_3_0->setChecked((bitmask & (1 << 0)) != 0);
    ui->checkBox_switch_0_3->setChecked((bitmask & (1 << 1)) != 0);
}

void MainWindow::initCharts()
{
    ValveInfo *valveInfo = m_registry->getValveInfo();
    bool rotate = (valveInfo->strokeMovement != 0);

    m_charts[Charts::Task] = ui->Chart_task;
    m_charts[Charts::Task]->setName(QStringLiteral("Task"));
    m_charts[Charts::Task]->useTimeaxis(false);
    m_charts[Charts::Task]->addAxis(QStringLiteral("%.2f bar"));
    if (!rotate) {
        m_charts[Charts::Task]->addAxis(QStringLiteral("%.2f mm"));
    } else {
        m_charts[Charts::Task]->addAxis(QStringLiteral("%.2f deg"));
    }
    m_charts[Charts::Task]->addSeries(1, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Task]->addSeries(1, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Task]->addSeries(0, tr("Датчик давления 1"), QColor::fromRgb(0, 0, 255));
    m_charts[Charts::Task]->addSeries(0, tr("Датчик давления 2"), QColor::fromRgb(0, 200, 0));
    m_charts[Charts::Task]->addSeries(0, tr("Датчик давления 3"), QColor::fromRgb(150, 0, 200));

    m_charts[Charts::Friction] = ui->Chart_friction;
    m_charts[Charts::Friction]->setName(QStringLiteral("Friction"));
    m_charts[Charts::Friction]->addAxis(QStringLiteral("%.2f H"));
    m_charts[Charts::Friction]->addSeries(0, tr("Трение от перемещения"), QColor::fromRgb(255, 0, 0));

    if (!rotate) {
        m_charts[Charts::Friction]->setLabelXformat(QStringLiteral("%.2f mm"));
    } else {
        m_charts[Charts::Friction]->setLabelXformat(QStringLiteral("%.2f deg"));
    }

    m_charts[Charts::Pressure] = ui->Chart_pressure;
    m_charts[Charts::Pressure]->setName(QStringLiteral("Pressure"));
    m_charts[Charts::Pressure]->useTimeaxis(false);
    m_charts[Charts::Pressure]->setLabelXformat(QStringLiteral("%.2f bar"));
    if (!rotate) {
        m_charts[Charts::Pressure]->addAxis(QStringLiteral("%.2f mm"));
    } else {
        m_charts[Charts::Pressure]->addAxis(QStringLiteral("%.2f deg"));
    }
    m_charts[Charts::Pressure]->addSeries(0, tr("Перемещение от давления"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Pressure]->addSeries(0, tr("Линейная регрессия"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Pressure]->visible(1, false);


    m_charts[Charts::Resolution] = ui->Chart_resolution;
    m_charts[Charts::Resolution]->setName(QStringLiteral("Resolution"));
    m_charts[Charts::Resolution]->useTimeaxis(true);
    m_charts[Charts::Resolution]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Resolution]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Resolution]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Response] = ui->Chart_response;
    m_charts[Charts::Response]->setName(QStringLiteral("Response"));
    m_charts[Charts::Response]->useTimeaxis(true);
    m_charts[Charts::Response]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Response]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Response]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Stroke] = ui->Chart_stroke;
    m_charts[Charts::Stroke]->setName(QStringLiteral("Stroke"));
    m_charts[Charts::Stroke]->useTimeaxis(true);
    m_charts[Charts::Stroke]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Stroke]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Stroke]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Step] = ui->Chart_step;
    m_charts[Charts::Step]->setName(QStringLiteral("Step"));
    m_charts[Charts::Step]->useTimeaxis(true);
    m_charts[Charts::Step]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Step]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Step]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));


    m_charts[Charts::Trend] = ui->Chart_trend;
    m_charts[Charts::Trend]->useTimeaxis(true);
    m_charts[Charts::Trend]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Trend]->addAxis(QStringLiteral("%.2f bar"));

    m_charts[Charts::Trend]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Trend]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));
    m_charts[Charts::Trend]->setMaxRange(60000);

    m_charts[Charts::Cyclic] = ui->Chart_cyclicTests;
    m_charts[Charts::Cyclic]->setName(QStringLiteral("Cyclic"));
    m_charts[Charts::Cyclic]->useTimeaxis(true);
    m_charts[Charts::Cyclic]->addAxis(QStringLiteral("%.2f%%"));
    m_charts[Charts::Cyclic]->addSeries(0, tr("Задание"), QColor::fromRgb(0, 0, 0));
    m_charts[Charts::Cyclic]->addSeries(0, tr("Датчик линейных перемещений"), QColor::fromRgb(255, 0, 0));
    // m_charts[Charts::Cyclic]->setMaxRange(80000);

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {
        m_charts[Charts::Cyclic]->addSeries(0, tr("Кв закрыто →"), QColor::fromRgb(200,200,0));
        m_charts[Charts::Cyclic]->addSeries(0, tr("Кв открыто →"), QColor::fromRgb(0,200,0));

        QChart *chart = m_charts[Charts::Cyclic]->chart();
        const auto all = chart->series();
        int total = all.size();

        for (int i = total - 4; i < total; ++i) {
            if (auto *xy = qobject_cast<QXYSeries*>(all.at(i))) {
                xy->setPointsVisible(true);
                QPen pen = xy->pen();
                pen.setStyle(Qt::NoPen);
                xy->setPen(pen);
                #if QT_VERSION >= QT_VERSION_CHECK(6,2,0)
                    xy->setMarkerSize(6.0);
                #endif
            }
        }
    }

    connect(m_program, &Program::addPoints, this,
            &MainWindow::addPoints);

    connect(m_program, &Program::clearPoints,
            this, &MainWindow::clearPoints);

    connect(m_program, &Program::duplicateMainChartsSeries,
            this, &MainWindow::duplicateMainChartsSeries);

    connect(m_program, &Program::setVisible,
            this, &MainWindow::setChartVisible);

    connect(m_program, &Program::setRegressionEnable,
            this, &MainWindow::setRegressionEnabled);

    connect(m_program, &Program::showDots,
            this, &MainWindow::showDots);

    connect(ui->checkBox_showCurve_task, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(0, state != 0);
    });

    connect(ui->checkBox_showCurve_moving, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(1, state != 0);
    });

    connect(ui->checkBox_showCurve_pressure_1, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(2, state != 0);
    });

    connect(ui->checkBox_showCurve_pressure_2, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(3, state != 0);
    });

    connect(ui->checkBox_showCurve_pressure_3, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Task]->visible(4, state != 0);
    });

    connect(ui->checkBox_regression, &QCheckBox::checkStateChanged,
            this, [&](int state) {
        m_charts[Charts::Pressure]->visible(1, state != 0);
    });

    connect(m_program, &Program::getPoints_mainTest,
            this, &MainWindow::onMainTestPointsRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getPoints_stepTest,
            this, &MainWindow::onStepTestPointsRequested,
            Qt::BlockingQueuedConnection);

    connect(m_program, &Program::getPoints_cyclicTest,
            this, &MainWindow::onCyclicTestPointsRequested,
            Qt::QueuedConnection);
}

void MainWindow::saveChart(Charts chart)
{
    m_reportSaver->saveImage(m_charts[chart]);

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
    case Charts::Cyclic:
        break;
    default:
        break;
    }

    ui->label_imageChartTask->setScaledContents(true);
    ui->label_imageChartPressure->setScaledContents(true);
    ui->label_imageChartFriction->setScaledContents(true);
}

void MainWindow::getImage(QLabel *label, QImage *image)
{
    QString imgPath = QFileDialog::getOpenFileName(this,
                                                   tr("Выберите файл"),
                                                   m_reportSaver->directory().absolutePath(),
                                                   tr("Изображения (*.jpg *.png *.bmp)"));

    if (!imgPath.isEmpty()) {
        QImage img(imgPath);
        *image = img.scaled(1000, 430, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(QPixmap::fromImage(img));
    }
}

void MainWindow::on_pushButton_init_clicked()
{
    QVector<bool> states = {
        ui->pushButton_DO0->isChecked(),
        ui->pushButton_DO1->isChecked(),
        ui->pushButton_DO2->isChecked(),
        ui->pushButton_DO3->isChecked()
    };

    emit doInitStatesSelected(states);
    emit initialized();
    emit patternChanged(m_patternType);
}

void MainWindow::on_pushButton_imageChartTask_clicked()
{
    getImage(ui->label_imageChartTask, &m_imageChartTask);
}
void MainWindow::on_pushButton_imageChartPressure_clicked()
{
    getImage(ui->label_imageChartPressure, &m_imageChartPressure);
}
void MainWindow::on_pushButton_imageChartFriction_clicked()
{
    getImage(ui->label_imageChartFriction, &m_imageChartFriction);
}

void MainWindow::on_pushButton_report_generate_clicked()
{
    std::unique_ptr<ReportBuilder> reportBuilder;

    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT: reportBuilder = std::make_unique<ReportBuilder_B_CVT>(); break;
    case SelectTests::Pattern_B_SACVT: reportBuilder = std::make_unique<ReportBuilder_B_SACVT>(); break;
    case SelectTests::Pattern_C_CVT: reportBuilder = std::make_unique<ReportBuilder_C_CVT>(); break;
    case SelectTests::Pattern_C_SACVT: reportBuilder = std::make_unique<ReportBuilder_C_SACVT>(); break;
    case SelectTests::Pattern_C_SOVT: reportBuilder = std::make_unique<ReportBuilder_C_SOVT>(); break;
    default:
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбран корректный паттерн отчёта!"));
        return;
    }

    ReportSaver::Report report;
    reportBuilder->buildReport(report,
                               m_telemetryStore,
                               *m_registry->getObjectInfo(),
                               *m_registry->getValveInfo(),
                               *m_registry->getOtherParameters(),
                               m_imageChartTask, m_imageChartPressure, m_imageChartFriction, m_imageChartStep);

    qDebug() << "Путь к шаблону:" << reportBuilder->templatePath();

    bool saved = m_reportSaver->saveReport(report, reportBuilder->templatePath());
    ui->pushButton_report_open->setEnabled(saved);
}
void MainWindow::on_pushButton_report_open_clicked()
{
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(m_reportSaver->directory().filePath(QStringLiteral("report.xlsx"))));
}
