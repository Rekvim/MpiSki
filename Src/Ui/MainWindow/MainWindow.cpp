#include "MainWindow.h"
#include "./Src/CustomChart/MyChart.h"
#include "ui_MainWindow.h"
#include "../Setup/ValveWindow/ValveWindow.h"

#include "Src/Utils/Shortcuts/TabBinder.h"
#include "Src/Utils/NumberUtils.h"

#include "Src/ReportBuilders/ReportBuilderFactory.h"
#include "Src/Ui/TestSettings/AbstractTestSettings.h"

namespace {
constexpr auto kArrowButtonStyle =
    "QToolButton {"
    "   background-color: transparent;"
    "   border: none;"
    "   padding: 0px;"
    "   margin: 0px;"
    "}"
    "QToolButton:hover {"
    "   background-color: transparent;"
    "}"
    "QToolButton:pressed {"
    "   background-color: transparent;"
    "}";

static QString formatRange(double lo, double hi)
{
    if (lo > hi) std::swap(lo, hi);
    return QString("%1–%2")
        .arg(lo, 0, 'f', 2)
        .arg(hi, 0, 'f', 2);
}

void setNum(QLineEdit* le, double v)
{
    le->setText(QString::number(v, 'f', 2));
}

void setPlusMinusPercent(QLineEdit* loLe, QLineEdit* hiLe,
                         double base, double pct)
{
    const double d = std::abs(base) * (pct / 100.0);
    setNum(loLe, base - d);
    setNum(hiLe, base + d);
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupUiConnections();

    m_mapper = std::make_unique<TelemetryUiMapper>(ui);
    m_chartManager = std::make_unique<ChartManager>(this);
    m_crossingIndicators = std::make_unique<CrossingIndicatorsPresenter>(ui);

    QHash<QWidget*, QTabWidget*> innerTabs;

    innerTabs[ui->tab_mainTests] = ui->tabWidget_mainTests;
    innerTabs[ui->tab_optionalTests] = ui->tabWidget_optionalTests;
    innerTabs[ui->tab_reportGeneration] = ui->tabWidget_reportGeneration;

    TabBinder::bindNumbers(this, ui->tabWidget_main);

    TabBinder::bindArrowNavigation(
        this,
        ui->tabWidget_main,
        innerTabs
    );

    setupPrimaryActions();
    setupShortcuts();

    auto* s = qobject_cast<LabeledSlider*>(ui->verticalSlider_task);

    if (s) {
        QTimer::singleShot(0, s, [s]{
            s->setFixedWidth(s->sizeHint().width());
        });
    }

    ui->tabWidget_main->setCurrentIndex(0);

    lockTabsForPreInit();

    m_mainTestSettings = new MainTestSettings(this);
    m_stepTestSettings = new StepTestSettings(this);
    m_responseTestSettings = new OtherTestSettings(this);
    m_resolutionTestSettings = new OtherTestSettings(this);
    m_cyclicTestSettings = new CyclicTestSettings(this);

    m_testSettings = {
        m_stepTestSettings,
        m_responseTestSettings,
        m_resolutionTestSettings,
        m_cyclicTestSettings
    };

    m_reportSaver = new ReportSaver(this);
    m_chartImages = new ChartImageService(
        m_chartManager.get(),
        m_reportSaver);


    ui->checkBox_switch_3_0->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_switch_0_3->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_lineEdits = {
        {TextObjects::LineEdit_linearSensor, ui->lineEdit_linearSensor},
        {TextObjects::LineEdit_linearSensorPercent, ui->lineEdit_linearSensorPercent},
        {TextObjects::LineEdit_pressureSensor_1, ui->lineEdit_pressureSensor_1},
        {TextObjects::LineEdit_pressureSensor_2, ui->lineEdit_pressureSensor_2},
        {TextObjects::LineEdit_pressureSensor_3, ui->lineEdit_pressureSensor_3},
        {TextObjects::LineEdit_feedback_4_20mA, ui->lineEdit_feedback_4_20mA}
    };

    m_program = new Program;
    m_programThread = new QThread(this);
    m_program->moveToThread(m_programThread);
    m_programThread->start();

    m_testController = new TestController(this);
    m_testController->setProgram(m_program);

    connect(m_testController, &TestController::startMainRequested,
            m_program, &Program::startMainTest);

    connect(m_testController, &TestController::startStrokeRequested,
            m_program, &Program::startStrokeTest);

    connect(m_testController, &TestController::startResponseRequested,
            m_program, &Program::startResponseTest);

    connect(m_testController, &TestController::startResolutionRequested,
            m_program, &Program::startResolutionTest);

    connect(m_testController, &TestController::startStepRequested,
            m_program, &Program::startStepTest);

    connect(m_testController, &TestController::startCyclicRequested,
            m_program, &Program::startCyclicTest);

    // kоговое окно
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
    m_durationTimer->setInterval(1000);

    connect(m_durationTimer, &QTimer::timeout,
            this, &MainWindow::onCountdownTimeout);

    connect(m_program, &Program::totalTestTimeMs,
            this, &MainWindow::onTotalTestTimeMs);

    connect(this, &MainWindow::initialized,
            m_program, &Program::initialization);

    connect(this, &MainWindow::doInitStatesSelected,
            m_program, &Program::setInitDoStates);

    connect(ui->pushButton_set, &QPushButton::clicked,
            m_program, &Program::button_set_position);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            m_program, &Program::checkbox_autoInit);

    connect(this, &MainWindow::setDo,
            m_program, &Program::button_DO);

    QPushButton* buttons[] = {
        ui->pushButton_DO0,
        ui->pushButton_DO1,
        ui->pushButton_DO2,
        ui->pushButton_DO3
    };

    for (int i = 0; i < 4; ++i) {
        connect(buttons[i], &QPushButton::clicked,
                this, [this, i](bool checked) {
                    emit setDo(i, checked);
                });
    }

    connect(this, &MainWindow::stopTest,
            m_testController, &TestController::stop);

    connect(m_program, &Program::testFinished,
            this, &MainWindow::endTest);

    connect(m_program, &Program::setText,
            this, &MainWindow::setText);

    connect(m_program, &Program::setDoButtonsChecked,
            this, &MainWindow::setDoButtonsChecked);

    connect(m_program, &Program::setDiCheckboxesChecked,
            this, &MainWindow::setDiCheckboxesChecked);

    connect(this, &MainWindow::dacValueRequested,
            m_program, &Program::setDacReal);

    connect(ui->doubleSpinBox_task,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, [this](double value) {
                if (qRound(value * 1000) != ui->verticalSlider_task->value()) {
                    if (ui->verticalSlider_task->isEnabled())
                        emit dacValueRequested(value);
                    ui->verticalSlider_task->setValue(qRound(value * 1000));
                }
            });

    connect(ui->verticalSlider_task, &QSlider::valueChanged,
            this, [this](int value) {
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

    connect(m_program, &Program::setSensorsMask,
            this, &MainWindow::setSensorsMask);

    connect(m_program, &Program::setButtonInitEnabled,
            this, &MainWindow::setButtonInitEnabled);

    connect(m_program, &Program::setTaskControlsEnabled,
            this, &MainWindow::setTaskControlsEnabled);

    connect(m_program, &Program::setStepResults,
            this, &MainWindow::setStepTestResults);

    connect(m_program, &Program::question,
            this, &MainWindow::askQuestion,
            Qt::BlockingQueuedConnection);

    connect(m_reportSaver, &ReportSaver::question,
            this, &MainWindow::askQuestion,
            Qt::DirectConnection);

    connect(m_reportSaver, &ReportSaver::setDirectoryToSave,
            this, &MainWindow::directoryToSave,
            Qt::DirectConnection);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                ui->pushButton_set->setEnabled(!state);
            });

    ui->tableWidget_stepResults->setColumnCount(2);
    ui->tableWidget_stepResults->setHorizontalHeaderLabels({QLatin1String("T86"), tr("Перерегулирование")});
    ui->tableWidget_stepResults->resizeColumnsToContents();

    setupArrowButton(ui->toolButton_arrowUp,
                     ":/Src/Img/arrowUp.png",
                     ":/Src/Img/arrowUpHover.png",
                     +0.05);

    setupArrowButton(ui->toolButton_arrowDown,
                     ":/Src/Img/arrowDown.png",
                     ":/Src/Img/arrowDownHover.png",
                     -0.05);

    connect(m_program, &Program::telemetryUpdated,
            this, &MainWindow::onTelemetryUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Program::cyclicCycleCompleted,
            this, [this](int completed){
                int remaining = completed;
                ui->label_cyclicTest_completedCyclesValue->setText(QString::number(remaining));
            });

    connect(m_testController, &TestController::stateChanged,
            this, &MainWindow::setTestState);

    ui->tabWidget_mainTests->setCurrentIndex(0);
    ui->tabWidget_optionalTests->setCurrentIndex(0);
    ui->tabWidget_reportGeneration->setCurrentIndex(0);

    connect(ui->tabWidget_main, &QTabWidget::currentChanged,
            this, [this](int) {
                ui->tabWidget_mainTests->setCurrentIndex(0);
                ui->tabWidget_optionalTests->setCurrentIndex(0);
                ui->tabWidget_reportGeneration->setCurrentIndex(0);
            });

    // bindImage
    auto bindImage = [this](QPushButton* btn, QLabel* label, QImage* img)
    {
        connect(btn, &QPushButton::clicked, this, [this, label, img]
                {
                    getImage(label, img);
                });
    };

    bindImage(ui->pushButton_imageChartTask,
              ui->label_imageChartTask,
              &m_imageChartTask);

    bindImage(ui->pushButton_imageChartPressure,
              ui->label_imageChartPressure,
              &m_imageChartPressure);

    bindImage(ui->pushButton_imageChartFriction,
              ui->label_imageChartFriction,
              &m_imageChartFriction);
}

MainWindow::~MainWindow()
{
    if (m_program) {
        QMetaObject::invokeMethod(
            m_program, "terminateTest",
            Qt::BlockingQueuedConnection);
    }

    m_programThread->quit();
    m_programThread->wait();

    delete ui;
}

void MainWindow::setupArrowButton(QToolButton* button,
                                  const QString& normalIcon,
                                  const QString& hoverIcon,
                                  double step)
{
    button->setProperty("normalIcon", normalIcon);
    button->setProperty("hoverIcon", hoverIcon);
    button->setProperty("step", step);

    button->setIcon(QIcon(normalIcon));
    button->setFixedSize(100, 60);
    button->setIconSize(QSize(90, 50));
    button->setText(QString());
    button->setAutoRepeat(true);
    button->setAutoRepeatDelay(300);
    button->setAutoRepeatInterval(100);
    button->setStyleSheet(QString::fromUtf8(kArrowButtonStyle));
    button->installEventFilter(this);

    connect(button, &QToolButton::clicked, this, [this, step]() {
        auto* spin = ui->doubleSpinBox_task;
        double next = spin->value() + step;
        next = std::clamp(next, spin->minimum(), spin->maximum());
        spin->setValue(next);
    });
}

void MainWindow::setupUiConnections()
{
    // ===== signal buttons =====
    connect(ui->pushButton_signal_4mA,
            &QPushButton::clicked,
            this, [this] { ui->doubleSpinBox_task->setValue(4.0); });

    connect(ui->pushButton_signal_8mA,
            &QPushButton::clicked,
            this, [this] { ui->doubleSpinBox_task->setValue(8.0); });

    connect(ui->pushButton_signal_12mA,
            &QPushButton::clicked,
            this, [this] { ui->doubleSpinBox_task->setValue(12.0); });

    connect(ui->pushButton_signal_16mA,
            &QPushButton::clicked,
            this, [this] { ui->doubleSpinBox_task->setValue(16.0); });

    connect(ui->pushButton_signal_20mA,
            &QPushButton::clicked,
            this, [this] { ui->doubleSpinBox_task->setValue(20.0); });

    // ===== main test =====
    connect(ui->pushButton_mainTest_start, &QPushButton::clicked,
            this, &MainWindow::startMainTestClicked);

    connect(ui->pushButton_mainTest_save, &QPushButton::clicked,
            this, &MainWindow::saveMainTestChartClicked);

    // ===== stroke test =====
    connect(ui->pushButton_strokeTest_start, &QPushButton::clicked,
            this, &MainWindow::startStrokeTestClicked);

    connect(ui->pushButton_strokeTest_save, &QPushButton::clicked,
            this, &MainWindow::saveStrokeChartClicked);


    // ===== optional tests =====
    connect(ui->pushButton_optionalTests_start, &QPushButton::clicked,
            this, &MainWindow::startOptionalTestClicked);
    connect(ui->pushButton_optionalTests_save, &QPushButton::clicked,
            this, &MainWindow::saveOptionalTestChartClicked);

    // ===== cyclic =====
    connect(ui->pushButton_cyclicTest_start, &QPushButton::clicked,
            this, &MainWindow::startCyclicTestClicked);

    connect(ui->pushButton_cyclicTest_save, &QPushButton::clicked,
            this, &MainWindow::saveCyclicChartClicked);

    // ===== init =====
    connect(ui->pushButton_init, &QPushButton::clicked,
            this, &MainWindow::initClicked);

    // ===== report =====
    connect(ui->pushButton_report_generate, &QPushButton::clicked,
            this, &MainWindow::generateReportClicked);

    connect(ui->pushButton_report_open,
            &QPushButton::clicked,
            this, &MainWindow::openReportClicked);

    // ===== navigation =====
    connect(ui->pushButton_back, &QPushButton::clicked,
            this, &MainWindow::backClicked);
}

void MainWindow::lockTabsForPreInit()
{
    // ui->tabWidget_main->setTabEnabled(ui->tabWidget_main->indexOf(ui->tab_mainTests), false);
    // ui->tabWidget_main->setTabEnabled(1, false);
    // ui->tabWidget_main->setTabEnabled(2, false);
    // ui->tabWidget_main->setTabEnabled(3, false);
    // ui->tabWidget_main->setTabEnabled(4, false);
}

void MainWindow::updateAvailableTabs()
{
    displayDependingPattern();
    if (!m_isInitialized) {
        lockTabsForPreInit();
        return;
    }
}

static QString formatHMS(quint64 ms)
{
    const quint64 totalSec = ms / 1000ULL;
    const quint64 h = totalSec / 3600ULL;
    const quint64 m = (totalSec % 3600ULL) / 60ULL;
    const quint64 s = totalSec % 60ULL;

    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void MainWindow::setupShortcuts()
{
    auto* enter = new QShortcut(QKeySequence(Qt::Key_Return), this);
    enter->setContext(Qt::ApplicationShortcut);

    connect(enter, &QShortcut::activated,
            this, [this] {
                m_tabActionRouter.triggerPrimary(
                    ui->tabWidget_main->currentWidget());
            });

    auto* shiftEnter =
        new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return), this);

    shiftEnter->setContext(Qt::ApplicationShortcut);

    connect(shiftEnter, &QShortcut::activated,
            this, [this] {
                m_tabActionRouter.triggerSecondary(
                    ui->tabWidget_main->currentWidget());
            });
}

void MainWindow::setupPrimaryActions()
{
    m_tabActionRouter.bindPrimary(
        ui->tab_manual,
        ui->pushButton_init);

    m_tabActionRouter.bindPrimary(
        ui->tab_strokeTest,
        ui->pushButton_strokeTest_start);

    m_tabActionRouter.bindPrimary(
        ui->tab_mainTests,
        ui->pushButton_mainTest_start);

    m_tabActionRouter.bindPrimary(
        ui->tab_optionalTests,
        ui->pushButton_optionalTests_start);

    m_tabActionRouter.bindPrimary(
        ui->tab_cyclicTests,
        ui->pushButton_cyclicTest_start);

    m_tabActionRouter.bindPrimary(
        ui->tab_reportGeneration,
        ui->pushButton_report_generate);

    m_tabActionRouter.bindSecondary(
        ui->tab_reportGeneration,
        ui->pushButton_report_open);
}

void MainWindow::onCountdownTimeout()
{
    const qint64 elapsed = m_elapsedTimer.elapsed();
    qint64 remaining = static_cast<qint64>(m_totalTestMs) - elapsed;
    if (remaining < 0) remaining = 0;

    ui->statusbar->showMessage(
        tr("Тест в процессе. До завершения теста осталось: %1 (прошло %2 из %3)")
            .arg(formatHMS(static_cast<quint64>(remaining)),
                 formatHMS(static_cast<quint64>(elapsed)),
                 formatHMS(m_totalTestMs))
        );

    if (remaining == 0)
        m_durationTimer->stop();
}

void MainWindow::onTotalTestTimeMs(quint64 totalMs)
{
    m_totalTestMs = totalMs;
    m_elapsedTimer.restart();

    ui->statusbar->showMessage(
        tr("Плановая длительность теста: %1").arg(formatHMS(m_totalTestMs))
        );

    m_durationTimer->setInterval(1000);
    m_durationTimer->start();
    onCountdownTimeout();
}

void MainWindow::applyCrossingLimitsFromRecommend(const ValveInfo& valveInfo)
{
    const CrossingLimits& limits = valveInfo.crossingLimits;

    if (limits.valveStrokeEnabled) {
        bool ok = false;
        const double stroke = NumberUtils::toDouble(valveInfo.valveStroke, &ok);
        if (ok) {
            setPlusMinusPercent(ui->lineEdit_crossingLimits_range_lowerLimit,
                                ui->lineEdit_crossingLimits_range_upperLimit,
                                stroke, limits.valveStroke);
        }
    }

    if (limits.springEnabled) {

        // Берём значения из чисел (как ты и хотел)
        double low = valveInfo.driveRangeLow;
        double high = valveInfo.driveRangeHigh;

        // Если по смыслу эти величины не могут быть отрицательными —
        // нормализуем (иначе получишь -1.28 и т.п.)
        low = std::abs(low);
        high = std::abs(high);

        if (low > high)
            std::swap(low, high);

        // Допуски ВОКРУГ каждого числа отдельно (как было у тебя)
        const double lowDelta = low * (limits.springLower / 100.0);
        const double highDelta = high * (limits.springUpper / 100.0);

        double lowLo = low - lowDelta;
        double lowHi = low + lowDelta;

        double highLo = high - highDelta;
        double highHi = high + highDelta;

        // Запрещаем отрицательные границы (если физически не бывает < 0)
        lowLo = std::max(0.0, lowLo);
        highLo = std::max(0.0, highLo);

        // На всякий случай порядок
        if (lowLo > lowHi) std::swap(lowLo, lowHi);
        if (highLo > highHi) std::swap(highLo, highHi);

        ui->lineEdit_crossingLimits_spring_lowerLimit->setText(formatRange(lowLo, lowHi));
        ui->lineEdit_crossingLimits_spring_upperLimit->setText(formatRange(highLo, highHi));
    }

    if (limits.dynamicErrorEnabled) {
        ui->lineEdit_crossingLimits_dynamicError_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_dynamicError_upperLimit->setText(valveInfo.dinamicErrorRecomend);
    }
}

void MainWindow::onTelemetryUpdated(const Telemetry &t) {
    qDebug() << "UPDATED telemetry ranges:"
             << t.cyclicTestRecord.regulatoryResult.ranges.size();

    m_telemetry = t;

    qDebug() << "STORED telemetry ranges:"
             << m_telemetry.cyclicTestRecord.regulatoryResult.ranges.size();
    m_mapper->updateInit(t.init);

    m_mapper->updateMainTest(t);
    m_mapper->updateCrossing(t);
    m_crossingIndicators->update(m_telemetry.crossingStatus);
    if (t.stroke)
        m_mapper->updateStrokeTest(*t.stroke);
}

void MainWindow::appendLog(const QString& text) {
    const QString stamp = QDateTime::currentDateTime()
    .toString("[hh:mm:ss.zzz] ");
    m_logOutput->appendPlainText(stamp + text);
}

// !!!
void MainWindow::setTaskControlsEnabled(bool enabled)
{
    ui->verticalSlider_task->setEnabled(enabled);
    ui->doubleSpinBox_task->setEnabled(enabled);
    ui->groupBox_DO->setEnabled(enabled);
    ui->groupBox_settingCurrentSignal->setEnabled(enabled);
    ui->pushButton_back->setEnabled(enabled);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    auto* button = qobject_cast<QToolButton*>(watched);
    if (button &&
        (button == ui->toolButton_arrowUp || button == ui->toolButton_arrowDown)) {

        const QString normalIcon = button->property("normalIcon").toString();
        const QString hoverIcon  = button->property("hoverIcon").toString();

        if (event->type() == QEvent::Enter) {
            button->setIcon(QIcon(hoverIcon));
            return true;
        }

        if (event->type() == QEvent::Leave) {
            button->setIcon(QIcon(normalIcon));
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setRegistry(Registry *registry)
{
    m_registry = registry;

    const auto& objectInfo = m_registry->objectInfo();
    const auto& valveInfo = m_registry->valveInfo();
    const auto& otherParameters = m_registry->otherParameters();
    const CrossingLimits &limits = valveInfo.crossingLimits;

    ui->lineEdit_date->setText(otherParameters.date);

    ui->lineEdit_object->setText(objectInfo.object);
    ui->lineEdit_manufacture->setText(objectInfo.manufactory);
    ui->lineEdit_department->setText(objectInfo.department);
    ui->lineEdit_FIO->setText(objectInfo.FIO);

    ui->lineEdit_positionNumber->setText(valveInfo.positionNumber);
    ui->lineEdit_manufacturer->setText(valveInfo.manufacturer);
    ui->lineEdit_valveModel->setText(valveInfo.valveModel);
    ui->lineEdit_serialNumber->setText(valveInfo.serialNumber);
    ui->lineEdit_DNPN->setText(QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    ui->lineEdit_driveModel->setText(valveInfo.driveModel);
    ui->lineEdit_positionerModel->setText(valveInfo.positionerModel);
    ui->lineEdit_strokeMovement->setText(otherParameters.strokeMovement);
    ui->lineEdit_safePosition->setText(otherParameters.safePosition);
    ui->lineEdit_resultsTable_dynamicErrorRecomend->setText(valveInfo.dinamicErrorRecomend);
    ui->lineEdit_materialStuffingBoxSeal->setText(ValveEnums::StuffingBoxSealToString(valveInfo.materialStuffingBoxSeal));

    const bool anyCrossingEnabled =
        limits.frictionEnabled
        || limits.linearCharacteristicEnabled
        || limits.valveStrokeEnabled
        || limits.springEnabled
        || limits.dynamicErrorEnabled;

    ui->groupBox_crossingLimits->setVisible(anyCrossingEnabled);

    ui->lineEdit_resultsTable_strokeRecomend->setText(valveInfo.valveStroke);

    if (valveInfo.driveType == DriveType::DoubleActing) {
        ui->lineEdit_resultsTable_driveRangeRecomend->setText(tr("Привод ДД"));
        ui->lineEdit_resultsTable_driveRangeReal->setText(tr("Привод ДД"));
    } else {
        ui->lineEdit_resultsTable_driveRangeRecomend->setText(
            QString("%1–%2")
                .arg(valveInfo.driveRangeLow, 0, 'f', 2)
                .arg(valveInfo.driveRangeHigh, 0, 'f', 2)
            );
    }

    ui->widget_crossingLimits_frictionForce->setVisible(limits.frictionEnabled);
    ui->widget_crossingLimits_linearCharacteristic->setVisible(limits.linearCharacteristicEnabled);
    ui->widget_crossingLimits_range->setVisible(limits.valveStrokeEnabled);
    ui->widget_crossingLimits_spring->setVisible(limits.springEnabled);
    ui->widget_crossingLimits_dynamicError->setVisible(limits.dynamicErrorEnabled);

    if (limits.frictionEnabled) {
        ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->setText(
            QString::number(limits.frictionCoefLower, 'f', 2));
        ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->setText(
            QString::number(limits.frictionCoefUpper, 'f', 2));
    }

    if (limits.linearCharacteristicEnabled) {
        ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_linearCharacteristic_upperLimit->setText(
            QString::number(limits.linearCharacteristic, 'f', 2));
    }

    applyCrossingLimitsFromRecommend(valveInfo);

    if (limits.dynamicErrorEnabled) {
        ui->lineEdit_crossingLimits_dynamicError_lowerLimit->setText(QStringLiteral("0"));
        ui->lineEdit_crossingLimits_dynamicError_upperLimit->setText(valveInfo.dinamicErrorRecomend);
    }

    for (AbstractTestSettings* s : m_testSettings)
        s->applyValveInfo(valveInfo);

    if (!m_chartsInitialized) {
        initCharts();
        m_chartsInitialized = true;
    }

    m_program->setRegistry(registry);

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
        ui->tabWidget_main->setTabEnabled(1, true);
        ui->tabWidget_main->setTabEnabled(2, false);
        ui->tabWidget_main->setTabEnabled(3, false);
        ui->tabWidget_main->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_B_SACVT:
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget_main->setTabEnabled(1, true);
        ui->tabWidget_main->setTabEnabled(2, false);
        ui->tabWidget_main->setTabEnabled(3, false);
        ui->tabWidget_main->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_CVT:
        ui->groupBox_DO->setVisible(false);
        ui->tabWidget_main->setTabEnabled(1, true);
        ui->tabWidget_main->setTabEnabled(2, true);
        ui->tabWidget_main->setTabEnabled(3, true);
        ui->tabWidget_main->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_SACVT:
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget_main->setTabEnabled(1, true);
        ui->tabWidget_main->setTabEnabled(2, true);
        ui->tabWidget_main->setTabEnabled(3, true);
        ui->tabWidget_main->setTabEnabled(4, true);
        break;
    case SelectTests::Pattern_C_SOVT:
        ui->groupBox_settingCurrentSignal->setVisible(false);
        ui->groupBox_DO->setEnabled(true);
        ui->tabWidget_main->setTabEnabled(1, true);
        ui->tabWidget_main->setTabEnabled(2, false);
        ui->tabWidget_main->setTabEnabled(3, false);
        ui->tabWidget_main->setTabEnabled(4, true);
        break;
    default:
        break;
    }
}

void MainWindow::setSensorsNumber(quint8 sensorCount)
{
    const bool hasSensors = (sensorCount > 0);

    if (hasSensors) m_isInitialized = true;

    if (m_testState == TestState::Idle)
        showIdleState();

    updateAvailableTabs();

    ui->groupBox_settingCurrentSignal->setEnabled(hasSensors);

    ui->pushButton_mainTest_start->setEnabled(sensorCount > 1);
    ui->pushButton_strokeTest_start->setEnabled(hasSensors);
    ui->pushButton_optionalTests_start->setEnabled(hasSensors);
    ui->pushButton_cyclicTest_start->setEnabled(hasSensors);

    ui->doubleSpinBox_task->setEnabled(hasSensors);
    ui->verticalSlider_task->setEnabled(hasSensors);

    displayDependingPattern();
}
void MainWindow::setButtonInitEnabled(bool enable)
{
    ui->pushButton_init->setEnabled(enable);
}

void MainWindow::onStrokeTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_chartManager->getPoints(chart, 1);
    QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_chartManager->getPoints(chart, 0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
}

void MainWindow::onMainTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_chartManager->getPoints(chart, 1);
    QPair<QList<QPointF>, QList<QPointF>> pointsPressure = m_chartManager->getPoints(Charts::Pressure, 0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsLinear.second.begin(), pointsLinear.second.end()});
    points.push_back({pointsPressure.first.begin(), pointsPressure.first.end()});
    points.push_back({pointsPressure.second.begin(), pointsPressure.second.end()});
}
void MainWindow::onStepTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_chartManager->getPoints(chart, 1);
    QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_chartManager->getPoints(chart, 0);

    points.clear();
    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
}
void MainWindow::onCyclicTestPointsRequested(QVector<QVector<QPointF>> &points, Charts chart)
{
    points.clear();

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        QPair<QList<QPointF>, QList<QPointF>> opened = m_chartManager->getPoints(chart, 3);
        QPair<QList<QPointF>, QList<QPointF>> closed = m_chartManager->getPoints(chart, 2);

        points.push_back({opened.first.begin(), opened.first.end()});
        points.push_back({closed.first.begin(), closed.first.end()});
    }

    QPair<QList<QPointF>, QList<QPointF>> pointsLinear = m_chartManager->getPoints(chart, 1);
    QPair<QList<QPointF>, QList<QPointF>> pointsTask = m_chartManager->getPoints(chart, 0);

    points.push_back({pointsLinear.first.begin(), pointsLinear.first.end()});
    points.push_back({pointsTask.first.begin(), pointsTask.first.end()});
}

void MainWindow::setRegressionEnabled(bool enabled)
{
    ui->checkBox_regression->setEnabled(enabled);
    ui->checkBox_regression->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
}

static QString seqToString(const QVector<qreal>& seq)
{
    QStringList parts;
    parts.reserve(seq.size());
    for (quint16 v : seq) parts << QString::number(v);
    return parts.join('-');
}

void MainWindow::onCyclicTestParametersRequested(CyclicTestParams &parameters)
{
    if (m_cyclicTestSettings->exec() != QDialog::Accepted) {
        parameters = {};
        return;
    }

    parameters = m_cyclicTestSettings->parameters();

    switch (parameters.type)
    {
    case CyclicTestParams::Regulatory:
    {
        const auto& p = parameters.regulatory;

        ui->label_cyclicTest_sequenceValue->setText(seqToString(p.sequence));
        ui->label_cyclicTest_specifiedCyclesValue->setText(QString::number(p.numCycles));

        if (p.enable20mA)
            ui->doubleSpinBox_task->setValue(20.0);

        break;
    }

    case CyclicTestParams::Shutoff:
    {
        const auto& p = parameters.shutoff;

        ui->label_cyclicTest_sequenceValue->setText(seqToString(p.sequence));
        ui->label_cyclicTest_specifiedCyclesValue->setText(QString::number(p.numCycles));

        break;
    }

    default:
        ui->label_cyclicTest_sequenceValue->clear();
        ui->label_cyclicTest_specifiedCyclesValue->clear();
        break;
    }

    qint64 totalMs = 0;

    switch (parameters.type)
    {
    case CyclicTestParams::Regulatory:
    {
        const auto& p = parameters.regulatory;

        quint64 steps = static_cast<quint64>(p.sequence.size()) * p.numCycles;

        totalMs = steps * (p.delayMs + p.holdMs);

        break;
    }

    case CyclicTestParams::Shutoff:
    {
        const auto& p = parameters.shutoff;

        quint64 steps = static_cast<quint64>(p.sequence.size()) * p.numCycles;

        totalMs = steps * (p.delayMs + p.holdMs);

        break;
    }

    case CyclicTestParams::Combined:
    {
        const auto& r = parameters.regulatory;
        const auto& s = parameters.shutoff;

        quint64 regSteps = static_cast<quint64>(r.sequence.size()) * r.numCycles;
        quint64 offSteps = static_cast<quint64>(s.sequence.size()) * s.numCycles;

        qint64 regMs = regSteps * (r.delayMs + r.holdMs);
        qint64 offMs = offSteps * (s.delayMs + s.holdMs);

        totalMs = regMs + offMs;

        break;
    }
    }

    QTime t0(0,0);
    t0 = t0.addMSecs(totalMs);

    ui->label_cyclicTest_totalTimeValue->setText(t0.toString("hh:mm:ss.zzz"));
}

bool MainWindow::askQuestion(const QString &title, const QString &text)
{
    return QMessageBox::question(this, title, text) == QMessageBox::Yes;
}

void MainWindow::directoryToSave(const QString &currentPath, QString &result)
{
    result = QFileDialog::getExistingDirectory(this,
                                               tr("Выберите папку для сохранения изображений"),
                                               currentPath);
}

void MainWindow::endTest()
{
    const TestState finalState = m_testState;

    if (m_durationTimer)
        m_durationTimer->stop();

    if (finalState == TestState::Finished) {
        promptSaveChartsAfterTest();
    }
}

void MainWindow::applyTestStateToUi(TestState state)
{
    switch (state) {
    case TestState::Idle:
        ui->statusbar->showMessage(tr("Готов к работе"));
        break;
    case TestState::Starting:
        ui->statusbar->showMessage(tr("Подготовка теста..."));
        break;
    case TestState::Running:
        ui->statusbar->showMessage(tr("Тест выполняется"));
        break;
    case TestState::Canceled:
        ui->statusbar->showMessage(tr("Тест остановлен"));
        break;
    case TestState::Finished:
        ui->statusbar->showMessage(tr("Сохранение результатов..."));
        QTimer::singleShot(1500, this, [this]{
            if (m_testState == TestState::Finished)
                showIdleState();
        });
        break;
    }
}

void MainWindow::showInitializingState()
{
    ui->statusbar->showMessage(tr("Инициализация устройства..."));
}

void MainWindow::showIdleState()
{
    ui->statusbar->showMessage(tr("Готов к работе"));
}

void MainWindow::setTestState(TestState state)
{
    if (m_testState == state)
        return;

    m_testState = state;
    applyTestStateToUi(state);
}

bool MainWindow::tryStartTest()
{
    if (m_testState == TestState::Running) {
        if (QMessageBox::question( this, tr("Внимание!"),
                tr("Вы действительно хотите завершить тест?"))
            == QMessageBox::Yes) {

            setTestState(TestState::Canceled);
            emit stopTest();
        } return false;
    } return true;
}

void MainWindow::startMainTestClicked()
{
    if (!tryStartTest())
        return;

    if (m_mainTestSettings->exec() != QDialog::Accepted)
        return;

    const auto params = m_mainTestSettings->parameters();

    m_testController->runMainTest(params);
}
void MainWindow::saveMainTestChartClicked()
{
    const auto *w = ui->tabWidget_mainTests->currentWidget();
    if (w == ui->tab_mainTests_task) {
        saveChart(Charts::Task);
    } else if (w == ui->tab_mainTests_pressure) {
        saveChart(Charts::Pressure);
    } else if (w == ui->tab_mainTests_friction) {
        saveChart(Charts::Friction);
    }
}

void MainWindow::promptSaveChartsAfterTest()
{
    const auto charts = chartsForCurrentTest();
    if (charts.isEmpty())
        return;

    auto answer = QMessageBox::question(
        this, tr("Сохранение результатов"),
        tr("Тест завершён.\nСохранить графики?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (answer != QMessageBox::Yes)
        return;

    for (Charts c : charts)
        saveChart(c);
}

QVector<Charts> MainWindow::chartsForCurrentTest() const
{
    QWidget* top = ui->tabWidget_main->currentWidget();

    if (top == ui->tab_strokeTest) {
        return { Charts::Stroke };
    }

    if (top == ui->tab_mainTests) {
        return { Charts::Task, Charts::Pressure, Charts::Friction };
    }

    if (top == ui->tab_optionalTests) {
        QWidget* w = ui->tabWidget_optionalTests->currentWidget();

        if (w == ui->tab_optionalTests_response)
            return { Charts::Response };

        if (w == ui->tab_optionalTests_resolution)
            return { Charts::Resolution };

        if (w == ui->tab_optionalTests_step)
            return { Charts::Step };
    }

    if (top == ui->tab_cyclicTests) {
        return { Charts::Cyclic };
    }

    return {};
}

void MainWindow::startStrokeTestClicked()
{
    if (!tryStartTest())
        return;

    m_testController->runStrokeTest();
}
void MainWindow::saveStrokeChartClicked()
{
    saveChart(Charts::Stroke);
}

void MainWindow::startOptionalTestClicked()
{
    if (!tryStartTest())
        return;

    const int id = ui->tabWidget_optionalTests->currentIndex();

    if (id == 0) {
        if (m_responseTestSettings->exec() != QDialog::Accepted)
            return;

        m_testController->runResponseTest(
            m_responseTestSettings->parameters());
    }
    else if (id == 1) {
        if (m_resolutionTestSettings->exec() != QDialog::Accepted)
            return;

        m_testController->runResolutionTest(
            m_resolutionTestSettings->parameters());
    }
    else if (id == 2) {
        if (m_stepTestSettings->exec() != QDialog::Accepted)
            return;

        m_testController->runStepTest(
            m_stepTestSettings->parameters());
    }
}
void MainWindow::saveOptionalTestChartClicked()
{
    const auto *w = ui->tabWidget_optionalTests->currentWidget();

    if (w == ui->tab_optionalTests_response) {
        saveChart(Charts::Response);
    } else if (w == ui->tab_optionalTests_resolution) {
        saveChart(Charts::Resolution);
    } else if (w == ui->tab_optionalTests_step) {
        saveChart(Charts::Step);
    }
}

void MainWindow::startCyclicTestClicked()
{
    if (!tryStartTest())
        return;

    m_cyclicTestSettings->applyPattern(m_patternType);

    if (m_cyclicTestSettings->exec() != QDialog::Accepted)
        return;

    m_testController->runCyclicTest(
        m_cyclicTestSettings->parameters());
}

void MainWindow::saveCyclicChartClicked()
{
    saveChart(Charts::Cyclic);
}

void MainWindow::setDoButtonsChecked(quint8 bitmask)
{
    const std::array<QPushButton*, 4> buttons = {
        ui->pushButton_DO0,
        ui->pushButton_DO1,
        ui->pushButton_DO2,
        ui->pushButton_DO3
    };

    for (int i = 0; i < 4; ++i) {
        QSignalBlocker blocker(buttons[i]);
        buttons[i]->setChecked((bitmask & (1 << i)) != 0);
    }

    ui->groupBox_DO->setEnabled(true);
}

void MainWindow::setDiCheckboxesChecked(quint8 bitmask)
{
    ui->checkBox_switch_3_0->setChecked((bitmask & (1 << 0)) != 0);
    ui->checkBox_switch_0_3->setChecked((bitmask & (1 << 1)) != 0);
}

void MainWindow::setSensorsMask(quint8 mask)
{
    const bool hasLinear = mask & (1 << 0);
    const bool hasP1 = mask & (1 << 1);
    const bool hasP2 = mask & (1 << 2);
    const bool hasP3 = mask & (1 << 3);

    ui->checkBox_showCurve_task->setVisible(hasLinear);
    ui->checkBox_showCurve_moving->setVisible(hasLinear);

    ui->checkBox_showCurve_pressure_1->setVisible(hasP1);
    ui->checkBox_showCurve_pressure_2->setVisible(hasP2);
    ui->checkBox_showCurve_pressure_3->setVisible(hasP3);

    ui->checkBox_showCurve_task->setChecked(hasLinear);
    ui->checkBox_showCurve_moving->setChecked(hasLinear);
    ui->checkBox_showCurve_pressure_1->setChecked(hasP1);
    ui->checkBox_showCurve_pressure_2->setChecked(hasP2);
    ui->checkBox_showCurve_pressure_3->setChecked(hasP3);

    syncTaskChartSeriesVisibility(mask);
}

void MainWindow::syncTaskChartSeriesVisibility(quint8 mask)
{
    auto *ch = m_chartManager->chart(Charts::Task);
    if (!ch) return;

    const bool hasLinear = mask & (1 << 0);
    const bool hasP1 = mask & (1 << 1);
    const bool hasP2 = mask & (1 << 2);
    const bool hasP3 = mask & (1 << 3);

    ch->visible(0, hasLinear && ui->checkBox_showCurve_task->isChecked());
    ch->visible(1, hasLinear && ui->checkBox_showCurve_moving->isChecked());

    ch->visible(2, hasP1 && ui->checkBox_showCurve_pressure_1->isChecked());
    ch->visible(3, hasP2 && ui->checkBox_showCurve_pressure_2->isChecked());
    ch->visible(4, hasP3 && ui->checkBox_showCurve_pressure_3->isChecked());
}

void MainWindow::initCharts()
{
    auto& valveInfo = m_registry->valveInfo();
    bool isRotaryStroke = (valveInfo.strokeMovement == StrokeMovement::Rotary);

    const QString strokeAxisFormat =
        isRotaryStroke ? QStringLiteral("%.2f deg")
                       : QStringLiteral("%.2f mm");

    const auto& colors = m_registry->sensorColors();

    m_chartManager->createTrendChart(
        ui->Chart_trend,
        colors.linear
    );
    m_chartManager->createStrokeChart(
        ui->Chart_stroke,
        colors.linear
    );
    m_chartManager->createTaskChart(
        ui->Chart_task,
        strokeAxisFormat,
        colors.linear,
        colors.pressure1,
        colors.pressure2,
        colors.pressure3
    );
    m_chartManager->createFrictionChart(
        ui->Chart_friction,
        strokeAxisFormat,
        colors.linear
    );
    m_chartManager->createPressureChart(
        ui->Chart_pressure,
        strokeAxisFormat,
        colors.linear
    );
    m_chartManager->createResponseChart(
        ui->Chart_response,
        colors.linear
    );
    m_chartManager->createResolutionChart(
        ui->Chart_resolution,
        colors.linear
    );
    m_chartManager->createStepChart(
        ui->Chart_step,
        colors.linear
    );

    auto* cyclic = m_chartManager->createCyclicChart(
        ui->Chart_cyclicTests,
        colors.linear
    );

    if (m_patternType == SelectTests::Pattern_C_SOVT ||
        m_patternType == SelectTests::Pattern_B_SACVT ||
        m_patternType == SelectTests::Pattern_C_SACVT) {

        cyclic->addSeries(0, tr("Кв закрыто →"), QColor(200, 200 ,0));
        cyclic->addSeries(0, tr("Кв открыто →"), QColor(0, 200, 0));

        cyclic->setSeriesMarkersOnly(2, true);
        cyclic->setSeriesMarkersOnly(3, true);
    }

    connect(m_program, &Program::addPoints,
            m_chartManager.get(), &ChartManager::addPoints);

    connect(m_program, &Program::clearPoints,
            m_chartManager.get(), &ChartManager::clearPoints);

    connect(m_program, &Program::duplicateMainChartsSeries,
            m_chartManager.get(), &ChartManager::duplicateMainChartsSeries);

    connect(m_program, &Program::setVisible,
            m_chartManager.get(), &ChartManager::setVisible);

    connect(m_program, &Program::setRegressionEnable,
            this, &MainWindow::setRegressionEnabled);

    connect(ui->checkBox_showCurve_task, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(Charts::Task)->visible(0, state != 0);
            });

    connect(ui->checkBox_showCurve_moving, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(Charts::Task)->visible(1, state != 0);
            });

    connect(ui->checkBox_showCurve_pressure_1, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(Charts::Task)->visible(2, state != 0);
            });

    connect(ui->checkBox_showCurve_pressure_2, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(Charts::Task)->visible(3, state != 0);
            });

    connect(ui->checkBox_showCurve_pressure_3, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(Charts::Task)->visible(4, state != 0);
            });

    connect(ui->checkBox_regression, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(Charts::Pressure)->visible(1, state != 0);
            });

    connect(m_program, &Program::getPoints_strokeTest,
            this, &MainWindow::onStrokeTestPointsRequested,
            Qt::BlockingQueuedConnection);

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

void MainWindow::initClicked()
{
    showInitializingState();

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

void MainWindow::restoreSeries(Charts chart, const SeriesVisibilityBackup& b)
{
    auto* ch = m_chartManager->chart(chart);
    if (!ch) return;

    if (chart == Charts::Task && b.visible.size() == 3) {
        ch->visible(2, b.visible[0]);
        ch->visible(3, b.visible[1]);
        ch->visible(4, b.visible[2]);
    }

    if (chart == Charts::Pressure && b.visible.size() == 1) {
        ch->visible(1, b.visible[0]);
    }
}

void MainWindow::saveChart(Charts chart)
{
    switch (chart) {
    case Charts::Task:
        m_chartImages->saveChart(
            chart,
            ui->label_imageChartTask,
            m_imageChartTask);
        break;
    case Charts::Pressure:
        m_chartImages->saveChart(
            chart,
            ui->label_imageChartPressure,
            m_imageChartPressure);
        break;
    case Charts::Friction:
        m_chartImages->saveChart(
            chart,
            ui->label_imageChartFriction,
            m_imageChartFriction);
        break;
    case Charts::Step:
        m_imageChartStep = m_chartImages->captureChart(chart);
        break;
    default:
        m_chartImages->saveChart(chart);
        break;
    }
}

void MainWindow::collectReportOverrides()
{
    // ===== MainTestRecord =====
    NumberUtils::readDouble(ui->lineEdit_resultsTable_frictionForceValue,
               m_telemetry.mainTestRecord.frictionForce);

    NumberUtils::readDouble(ui->lineEdit_resultsTable_frictionPercentValue,
               m_telemetry.mainTestRecord.frictionPercent);

    NumberUtils::readDouble(ui->lineEdit_resultsTable_dynamicErrorReal,
               m_telemetry.mainTestRecord.dynamicErrorReal);

    NumberUtils::readRange(ui->lineEdit_resultsTable_rangePressure,
              m_telemetry.mainTestRecord.lowLimitPressure,
              m_telemetry.mainTestRecord.highLimitPressure);

    NumberUtils::readRange(ui->lineEdit_resultsTable_driveRangeReal,
              m_telemetry.mainTestRecord.springLow,
              m_telemetry.mainTestRecord.springHigh);

    // ===== Stroke =====
    NumberUtils::readDouble(ui->lineEdit_resultsTable_strokeReal,
               m_telemetry.valveStrokeRecord.real);

    // ===== Stroke test =====
    if (m_telemetry.stroke)
    {
        m_telemetry.stroke->timeForwardMs =
            ui->lineEdit_resultsTable_strokeTest_forwardTime->text();

        m_telemetry.stroke->timeBackwardMs =
            ui->lineEdit_resultsTable_strokeTest_backwardTime->text();
    }

    // SupplyRecord
    m_telemetry.supplyRecord.pressure_bar =
        QString(ui->lineEdit_supplyPressure->text()).toDouble();
}

void MainWindow::collectRegistryOverrides(
    ObjectInfo& objectInfo,
    ValveInfo& valveInfo,
    OtherParameters& otherParameters)
{
    NumberUtils::readRange(ui->lineEdit_resultsTable_driveRangeRecomend,
              valveInfo.driveRangeLow,
              valveInfo.driveRangeHigh);

    valveInfo.dinamicErrorRecomend = ui->lineEdit_resultsTable_dynamicErrorRecomend->text();
    valveInfo.valveStroke = ui->lineEdit_resultsTable_strokeRecomend->text();
}

void MainWindow::generateReportClicked()
{
    collectReportOverrides();

    ObjectInfo objectInfo = m_registry->objectInfo();
    ValveInfo valveInfo = m_registry->valveInfo();
    OtherParameters otherParameters = m_registry->otherParameters();

    collectRegistryOverrides(objectInfo, valveInfo, otherParameters);

    auto reportBuilder = ReportBuilderFactory::create(m_patternType);

    if (!reportBuilder)
    {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбран корректный паттерн отчёта!"));
        return;
    }

    ReportSaver::Report report;
    reportBuilder->buildReport(report,
                               m_telemetry,
                               m_registry->objectInfo(),
                               m_registry->valveInfo(),
                               m_registry->otherParameters(),
                               m_imageChartTask, m_imageChartPressure, m_imageChartFriction, m_imageChartStep);

    bool saved = m_reportSaver->saveReport(report, reportBuilder->templatePath());
    ui->pushButton_report_open->setEnabled(saved);
}
void MainWindow::openReportClicked()
{
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(m_reportSaver->directory().filePath(QStringLiteral("report.xlsx"))));
}

void MainWindow::backClicked()
{
    if (m_testState == TestState::Running ||
        m_testState == TestState::Starting) {
        QMessageBox::warning(this,
                             tr("Внимание"),
                             tr("Нельзя вернуться во время выполнения теста"));
        return;
    }

    this->hide();

    ValveWindow valveWindow(this);
    valveWindow.setRegistry(m_registry);
    valveWindow.setPatternType(m_patternType);

    if (valveWindow.exec() == QDialog::Accepted) {
        setRegistry(m_registry);
    }

    this->show();
}
