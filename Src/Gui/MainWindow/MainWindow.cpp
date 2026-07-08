#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QImage>
#include <QtMath>
#include <QRandomGenerator>

#include "Gui/Setup/ValveWindow/ValveWindow.h"
#include "Gui/TestSettings/BaseSequenceSettingsDialog.h"
#include "Utils/Shortcuts/TabBinder.h"
#include "Utils/NumberUtils.h"
#include "Report/BuilderFactory.h"
#include "Widgets/Chart/ChartView.h"

#include "Widgets/Chart/ImageService.h"
#include "Domain/Tests/Main/Result.h"

using ChartType = Widgets::Chart::ChartType;

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
    m_chartManager = std::make_unique<Widgets::Chart::Manager>(this);
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

    auto* s = qobject_cast<Widgets::Slider::SliderView*>(ui->verticalSlider_task);

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

    m_reportSaver = new Report::Saver(this);
    m_chartImages = new Widgets::Chart::ImageService(
        m_chartManager.get(),
        m_reportSaver);


    ui->checkBox_switch_3_0->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_switch_0_3->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_program = new Domain::Program;
    m_programThread = new QThread(this);
    m_program->moveToThread(m_programThread);
    m_programThread->start();

    m_testController = new TestController(this);
    m_testController->setProgram(m_program);

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

    connect(m_program, &Domain::Program::totalTestTimeMs,
            this, &MainWindow::onTotalTestTimeMs);

    connect(this, &MainWindow::initialized,
            m_program, &Domain::Program::initialization);

    connect(this, &MainWindow::doInitStatesSelected,
            m_program, &Domain::Program::setInitDoStates);

    connect(ui->pushButton_set, &QPushButton::clicked,
            m_program, &Domain::Program::button_set_position);

    connect(ui->checkBox_autoinit, &QCheckBox::checkStateChanged,
            m_program, &Domain::Program::checkbox_autoInit);

    connect(this, &MainWindow::setDo,
            m_program, &Domain::Program::button_DO);

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

    connect(m_program, &Domain::Program::testFinished,
            this, &MainWindow::endTest);

    connect(m_program, &Domain::Program::setDoButtonsChecked,
            this, &MainWindow::setDoButtonsChecked);

    connect(m_program, &Domain::Program::setDiCheckboxesChecked,
            this, &MainWindow::setDiCheckboxesChecked);

    connect(m_program, &Domain::Program::sampleReady,
            this, &MainWindow::onSampleReady,
            Qt::QueuedConnection);

    connect(this, &MainWindow::dacValueRequested,
            m_program, &Domain::Program::setDacReal);

    connect(ui->doubleSpinBox_task, qOverload<double>(&QDoubleSpinBox::valueChanged),
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

    connect(this, &MainWindow::profileChanged,
            m_program, &Domain::Program::setProfile);

    connect(m_program, &Domain::Program::setTask,
            this, &MainWindow::setTask);

    connect(m_program, &Domain::Program::setSensorNumber,
            this, &MainWindow::setSensorsNumber);

    connect(m_program, &Domain::Program::setSensorsMask,
            this, &MainWindow::setSensorsMask);

    connect(m_program, &Domain::Program::mainResultUpdated,
            this, &MainWindow::onMainResultUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::strokeResultUpdated,
            this, &MainWindow::onStrokeResultUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::stepResultUpdated,
            this, &MainWindow::onStepResultUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::cyclicRegulatoryResultUpdated,
            this, &MainWindow::onCyclicRegulatoryResultUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::cyclicShutoffResultUpdated,
            this, &MainWindow::onCyclicShutoffResultUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::crossingStatusUpdated,
            this, &MainWindow::onCrossingStatusUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::question,
            this, &MainWindow::askQuestion,
            Qt::BlockingQueuedConnection);

    connect(m_reportSaver, &Report::Saver::question,
            this, &MainWindow::askQuestion,
            Qt::DirectConnection);

    connect(m_reportSaver, &Report::Saver::setDirectoryToSave,
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

    connect(m_program, &Domain::Program::telemetryUpdated,
            this, &MainWindow::onTelemetryUpdated,
            Qt::QueuedConnection);

    connect(m_program, &Domain::Program::cyclicCycleCompleted,
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
    auto bindImage = [this](QPushButton* btn, QLabel* label, ChartType chart)
    {
        connect(btn, &QPushButton::clicked, this, [this, label, chart]
                {
                    getImage(label, chart);
                });
    };

    bindImage(ui->pushButton_imageChartTask,
              ui->label_imageChartTask,
              ChartType::Task);

    bindImage(ui->pushButton_imageChartPressure,
              ui->label_imageChartPressure,
              ChartType::Pressure);

    bindImage(ui->pushButton_imageChartFriction,
              ui->label_imageChartFriction,
              ChartType::Friction);

    // ===== comments =====
    connect(ui->pushButton_commentMainTest,
            &QPushButton::clicked,
            this, [this] {
                editTestComment(
                    QStringLiteral("mainTest"),
                    tr("Основной тест") );
            });
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

void MainWindow::onSampleReady(const Domain::Measurement::Sample& s)
{
    if (!qIsNaN(s.positionValue))
        ui->lineEdit_linearSensor->setText(
            QString("%1 %2").arg(s.positionValue, 0, 'f', 2).arg(s.positionUnit));

    if (!qIsNaN(s.positionPercent))
        ui->lineEdit_linearSensorPercent->setText(
            QString("%1 %").arg(s.positionPercent, 0, 'f', 2));

    if (!qIsNaN(s.pressure1))
        ui->lineEdit_pressureSensor_1->setText(
            QString("%1 bar").arg(s.pressure1, 0, 'f', 2));

    if (!qIsNaN(s.pressure2))
        ui->lineEdit_pressureSensor_2->setText(
            QString("%1 bar").arg(s.pressure2, 0, 'f', 2));

    if (!qIsNaN(s.pressure3))
        ui->lineEdit_pressureSensor_3->setText(
            QString("%1 bar").arg(s.pressure3, 0, 'f', 2));

    if (!qIsNaN(s.feedbackCurrent))
        ui->lineEdit_feedback_4_20mA->setText(
            QString("%1 mA").arg(s.positionValue, 0, 'f', 2));
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

    connect(ui->pushButton_fakePressureData, &QPushButton::clicked,
            this, &MainWindow::generateFakePressureData);

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
    if (m_testState != TestState::Running &&
        m_testState != TestState::Starting) {
        if (m_durationTimer)
            m_durationTimer->stop();

        return;
    }

    const qint64 elapsed = m_elapsedTimer.elapsed();
    qint64 remaining = static_cast<qint64>(m_totalTestMs) - elapsed;
    if (remaining < 0)
        remaining = 0;

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
    if (m_testState != TestState::Starting &&
        m_testState != TestState::Running) {
        qDebug() << "MainWindow::onTotalTestTimeMs ignored, state =" << static_cast<int>(m_testState);
        return;
    }

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

void MainWindow::onMainResultUpdated(const Domain::Tests::Main::Result& result)
{
    m_telemetry.testMain = result;

    m_mapper->updateMainTest(result, m_telemetry.valveStrokeRecord);
    m_mapper->updateCrossingValues(result, m_telemetry.valveStrokeRecord);
}

void MainWindow::onStrokeResultUpdated(const Domain::Tests::Stroke::Result& result)
{
    m_telemetry.testStroke = result;
    m_mapper->updateStrokeTest(result);
}

void MainWindow::onStepResultUpdated(const Domain::Tests::Option::Step::Result& result)
{
    m_telemetry.testStep = result;
    m_mapper->updateStepTest(result);
}

void MainWindow::onCyclicRegulatoryResultUpdated(
    const Domain::Tests::Cyclic::Regulatory::Result& result)
{
    m_telemetry.testСyclicRegulatory = result;
    m_mapper->updateCyclicRegulatoryTest(result);
}

void MainWindow::onCyclicShutoffResultUpdated(
    const Domain::Tests::Cyclic::Shutoff::Result& result)
{
    m_telemetry.testСyclicShutoff = result;
    m_mapper->updateCyclicShutoffTest(result);
}

void MainWindow::onCrossingStatusUpdated(const CrossingStatus& status)
{
    m_telemetry.crossingStatus = status;
    m_crossingIndicators->update(status);
}

void MainWindow::onTelemetryUpdated(const Telemetry& t)
{
    m_telemetry = t;

    m_mapper->updateInit(t.init);

    if (t.testMain) {
        m_mapper->updateMainTest(*t.testMain, t.valveStrokeRecord);
        m_mapper->updateCrossingValues(*t.testMain, t.valveStrokeRecord);
    }

    if (t.testStroke)
        m_mapper->updateStrokeTest(*t.testStroke);

    if (t.testStep)
        m_mapper->updateStepTest(*t.testStep);

    if (t.testСyclicRegulatory)
        m_mapper->updateCyclicRegulatoryTest(*t.testСyclicRegulatory);

    if (t.testСyclicShutoff)
        m_mapper->updateCyclicShutoffTest(*t.testСyclicShutoff);

    m_crossingIndicators->update(t.crossingStatus);
}

void MainWindow::appendLog(const QString& text) {
    const QString stamp = QDateTime::currentDateTime()
    .toString("[hh:mm:ss.zzz] ");
    m_logOutput->appendPlainText(stamp + text);
}

// !!!
void MainWindow::setTaskControlsEnabled(bool enabled)
{
    ui->pushButton_init->setEnabled(enabled);
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

void MainWindow::setRegistry(Registry* registry)
{
    m_registry = registry;

    const auto& objectInfo = m_registry->objectInfo();
    const auto& valveInfo = m_registry->valveInfo();
    const auto& otherParameters = m_registry->otherParameters();
    const CrossingLimits &limits = valveInfo.crossingLimits;

    const QString basePath =
        QStringLiteral("%1/%2/%3/%4")
            .arg(objectInfo.object,
                 objectInfo.manufactory,
                 objectInfo.department,
                 valveInfo.positionNumber);

    m_reportSaver->setBasePath(basePath);

    Domain::DeviceConfig cfg;
    cfg.safePosition = valveInfo.safePosition;
    cfg.driveDiameter = valveInfo.driveDiameter;
    cfg.valveStroke = valveInfo.valveStroke;
    cfg.crossingLimits = valveInfo.crossingLimits;
    cfg.strokeMovement = valveInfo.strokeMovement;
    cfg.diameterPulley = valveInfo.diameterPulley;
    cfg.dinamicErrorRecomend = valveInfo.dinamicErrorRecomend.toDouble();
    cfg.driveRangeLow = valveInfo.driveRangeLow;
    cfg.driveRangeHigh = valveInfo.driveRangeHigh;

    m_program->setConfig(cfg);

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

    const auto& testSettings = m_testSettings;
    for (BaseSequenceSettingsDialog* s : testSettings)
        s->applyValveInfo(valveInfo);

    if (!m_chartsInitialized) {
        initCharts();
        m_chartsInitialized = true;
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

void MainWindow::displayDependingPattern()
{
    const bool hasShutoff = m_deviceProfile.hasShutoff();
    const bool hasControl = m_deviceProfile.hasControl();
    const bool isComplex = m_deviceProfile.isComplex();

    ui->groupBox_DO->setVisible(hasShutoff);
    ui->groupBox_settingCurrentSignal->setVisible(hasControl);

    ui->tabWidget_main->setTabEnabled(1, true);
    ui->tabWidget_main->setTabEnabled(2, isComplex && hasControl);
    ui->tabWidget_main->setTabEnabled(3, isComplex && hasControl);
    ui->tabWidget_main->setTabEnabled(4, true);

    applyProfileVisibility();
}

void MainWindow::applyProfileVisibility()
{
    ui->pushButton_DO0->setVisible(m_deviceProfile.do1);
    ui->pushButton_DO1->setVisible(m_deviceProfile.do2);
    ui->pushButton_DO2->setVisible(m_deviceProfile.do3);
    ui->pushButton_DO3->setVisible(m_deviceProfile.do4);

    ui->groupBox_pressureSensor_1->setVisible(m_deviceProfile.pressure1);
    ui->groupBox_pressureSensor_2->setVisible(m_deviceProfile.pressure2);
    ui->groupBox_pressureSensor_3->setVisible(m_deviceProfile.pressure3);
    ui->groupBox_feedback_4_20mA->setVisible(m_deviceProfile.outputSignal);
}

void MainWindow::setSensorsNumber(quint8 sensorCount)
{
    const bool hasSensors = (sensorCount > 0);

    if (hasSensors) {
        m_isInitialized = true;
        setTaskControlsEnabled(true);
    } else {
        setTaskControlsEnabled(false);
    }
    if (m_testState == TestState::Idle)
        showIdleState();

    updateAvailableTabs();

    ui->label_valveStroke_range->setText(m_telemetry.valveStrokeRecord.range);
    ui->lineEdit_crossingLimits_range_value->setText(m_telemetry.valveStrokeRecord.range);

    ui->pushButton_mainTest_start->setEnabled(sensorCount > 1);
    ui->pushButton_strokeTest_start->setEnabled(hasSensors);
    ui->pushButton_optionalTests_start->setEnabled(hasSensors);
    ui->pushButton_cyclicTest_start->setEnabled(hasSensors);

    ui->pushButton_init->setEnabled(true);
    displayDependingPattern();
}

void MainWindow::setRegressionEnabled(bool enabled)
{
    ui->checkBox_regression->setEnabled(enabled);
    ui->checkBox_regression->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

    auto* pressureChart = m_chartManager->chart(ChartType::Pressure);
    if (pressureChart)
        pressureChart->visible(2, enabled);
}

static QString seqToString(const QVector<qreal>& seq)
{
    QStringList parts;
    parts.reserve(seq.size());
    for (quint16 v : seq) parts << QString::number(v);
    return parts.join('-');
}

qint64 MainWindow::cyclicTotalTimeMs(
    const Domain::Tests::Cyclic::Params& parameters) const
{
    qint64 totalMs = 0;

    switch (parameters.type) {
    case Domain::Tests::Cyclic::Params::Regulatory:
    {
        const auto& p = parameters.regulatory;

        const quint64 steps =
            static_cast<quint64>(p.sequence.size()) * p.numCycles;

        totalMs = static_cast<qint64>(steps * (p.delayMs + p.holdMs));
        break;
    }

    case Domain::Tests::Cyclic::Params::Shutoff:
    {
        const auto& p = parameters.shutoff;

        const quint64 steps =
            static_cast<quint64>(p.sequence.size()) * p.numCycles;

        totalMs = static_cast<qint64>(steps * (p.delayMs + p.holdMs));
        break;
    }

    case Domain::Tests::Cyclic::Params::Combined:
    {
        const auto& r = parameters.regulatory;
        const auto& s = parameters.shutoff;

        const quint64 regSteps =
            static_cast<quint64>(r.sequence.size()) * r.numCycles;

        const quint64 offSteps =
            static_cast<quint64>(s.sequence.size()) * s.numCycles;

        const qint64 regMs =
            static_cast<qint64>(regSteps * (r.delayMs + r.holdMs));

        const qint64 offMs =
            static_cast<qint64>(offSteps * (s.delayMs + s.holdMs));

        totalMs = regMs + offMs;
        break;
    }
    }

    return totalMs;
}

void MainWindow::updateCyclicLabels(
    const Domain::Tests::Cyclic::Params& parameters)
{
    switch (parameters.type) {
    case Domain::Tests::Cyclic::Params::Regulatory:
    {
        const auto& p = parameters.regulatory;

        ui->label_cyclicTest_sequenceValue->setText(seqToString(p.sequence));
        ui->label_cyclicTest_specifiedCyclesValue->setText(QString::number(p.numCycles));

        break;
    }

    case Domain::Tests::Cyclic::Params::Shutoff:
    {
        const auto& p = parameters.shutoff;

        ui->label_cyclicTest_sequenceValue->setText(seqToString(p.sequence));
        ui->label_cyclicTest_specifiedCyclesValue->setText(QString::number(p.numCycles));

        break;
    }

    case Domain::Tests::Cyclic::Params::Combined:
    {
        const auto& r = parameters.regulatory;
        const auto& s = parameters.shutoff;

        ui->label_cyclicTest_sequenceValue->setText(
            QStringLiteral("Рег.: %1 | Отсеч.: %2")
                .arg(seqToString(r.sequence),
                     seqToString(s.sequence))
            );

        ui->label_cyclicTest_specifiedCyclesValue->setText(
            QStringLiteral("Рег.: %1 | Отсеч.: %2")
                .arg(r.numCycles)
                .arg(s.numCycles)
            );

        break;
    }
    }

    QTime t0(0, 0);
    t0 = t0.addMSecs(cyclicTotalTimeMs(parameters));

    ui->label_cyclicTest_totalTimeValue->setText(
        t0.toString(QStringLiteral("hh:mm:ss.zzz"))
        );
}

void MainWindow::onCyclicTestParametersRequested(Domain::Tests::Cyclic::Params &parameters)
{
    if (m_cyclicTestSettings->exec() != QDialog::Accepted) {
        parameters = {};
        return;
    }

    parameters = m_cyclicTestSettings->parameters();

    switch (parameters.type)
    {
    case Domain::Tests::Cyclic::Params::Regulatory:
    {
        const auto& p = parameters.regulatory;

        ui->label_cyclicTest_sequenceValue->setText(seqToString(p.sequence));
        ui->label_cyclicTest_specifiedCyclesValue->setText(QString::number(p.numCycles));

        if (p.enable20mA)
            ui->doubleSpinBox_task->setValue(20.0);

        break;
    }

    case Domain::Tests::Cyclic::Params::Shutoff:
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
    case Domain::Tests::Cyclic::Params::Regulatory:
    {
        const auto& p = parameters.regulatory;

        quint64 steps = static_cast<quint64>(p.sequence.size()) * p.numCycles;

        totalMs = steps * (p.delayMs + p.holdMs);

        break;
    }

    case Domain::Tests::Cyclic::Params::Shutoff:
    {
        const auto& p = parameters.shutoff;

        quint64 steps = static_cast<quint64>(p.sequence.size()) * p.numCycles;

        totalMs = steps * (p.delayMs + p.holdMs);

        break;
    }

    case Domain::Tests::Cyclic::Params::Combined:
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
        setTaskControlsEnabled(false);
        break;
    case TestState::Canceled:
        ui->statusbar->showMessage(tr("Тест остановлен"));
        setTaskControlsEnabled(true);
        break;
    case TestState::Finished:
        ui->statusbar->showMessage(tr("Сохранение результатов..."));
        setTaskControlsEnabled(true);
        QTimer::singleShot(1500, this, [this]{
            if (m_testState == TestState::Finished)
                showIdleState();
        });
        break;
    }
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
        saveChart(ChartType::Task);
    } else if (w == ui->tab_mainTests_pressure) {
        saveChart(ChartType::Pressure);
    } else if (w == ui->tab_mainTests_friction) {
        saveChart(ChartType::Friction);
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

    for (ChartType c : charts)
        saveChart(c);
}

QVector<ChartType> MainWindow::chartsForCurrentTest() const
{
    QWidget* top = ui->tabWidget_main->currentWidget();

    if (top == ui->tab_strokeTest) {
        return { ChartType::Stroke };
    }

    if (top == ui->tab_mainTests) {
        return { ChartType::Task, ChartType::Pressure, ChartType::Friction };
    }

    if (top == ui->tab_optionalTests) {
        QWidget* w = ui->tabWidget_optionalTests->currentWidget();

        if (w == ui->tab_optionalTests_response)
            return { ChartType::Response };

        if (w == ui->tab_optionalTests_resolution)
            return { ChartType::Resolution };

        if (w == ui->tab_optionalTests_step)
            return { ChartType::Step };
    }

    if (top == ui->tab_cyclicTests) {
        return { ChartType::Cyclic };
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
    saveChart(ChartType::Stroke);
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
        saveChart(ChartType::Response);
    } else if (w == ui->tab_optionalTests_resolution) {
        saveChart(ChartType::Resolution);
    } else if (w == ui->tab_optionalTests_step) {
        saveChart(ChartType::Step);
    }
}

void MainWindow::startCyclicTestClicked()
{
    if (!tryStartTest())
        return;

    m_cyclicTestSettings->applyProfile(m_deviceProfile);

    if (m_cyclicTestSettings->exec() != QDialog::Accepted)
        return;

    const auto parameters = m_cyclicTestSettings->parameters();

    updateCyclicLabels(parameters);

    if (parameters.type == Domain::Tests::Cyclic::Params::Regulatory) {
        if (parameters.regulatory.enable20mA)
            ui->doubleSpinBox_task->setValue(20.0);
    }

    m_testController->runCyclicTest(parameters);
}

void MainWindow::saveCyclicChartClicked()
{
    saveChart(ChartType::Cyclic);
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
    auto *ch = m_chartManager->chart(ChartType::Task);
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

    if (m_deviceProfile.hasShutoff()) {
        cyclic->addSeries(0, tr("Кв закрыто →"), QColor(200, 200 ,0));
        cyclic->addSeries(0, tr("Кв открыто →"), QColor(0, 200, 0));

        cyclic->setSeriesMarkersOnly(2, true);
        cyclic->setSeriesMarkersOnly(3, true);
    }

    connect(m_program, &Domain::Program::addPoints,
            m_chartManager.get(), &Widgets::Chart::Manager::addPoints);

    connect(m_program, &Domain::Program::clearPoints,
            m_chartManager.get(), &Widgets::Chart::Manager::clearPoints);

    connect(m_program, &Domain::Program::duplicateMainChartsSeries,
            m_chartManager.get(), &Widgets::Chart::Manager::duplicateMainChartsSeries);

    connect(m_program, &Domain::Program::setVisible,
            m_chartManager.get(), &Widgets::Chart::Manager::setVisible);

    connect(m_program, &Domain::Program::setRegressionEnable,
            this, &MainWindow::setRegressionEnabled);

    connect(ui->checkBox_showCurve_task, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(ChartType::Task)->visible(0, state != 0);
            });

    connect(ui->checkBox_showCurve_moving, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(ChartType::Task)->visible(1, state != 0);
            });

    connect(ui->checkBox_showCurve_pressure_1, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(ChartType::Task)->visible(2, state != 0);
            });

    connect(ui->checkBox_showCurve_pressure_2, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(ChartType::Task)->visible(3, state != 0);
            });

    connect(ui->checkBox_showCurve_pressure_3, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                m_chartManager->chart(ChartType::Task)->visible(4, state != 0);
            });

    connect(ui->checkBox_regression, &QCheckBox::checkStateChanged,
            this, [&](int state) {
                const bool on = (state != 0);
                m_chartManager->chart(ChartType::Pressure)->visible(1, on);
                m_chartManager->chart(ChartType::Pressure)->visible(2, on);
            });

    connect(m_chartManager->chart(ChartType::Pressure),
            &Widgets::Chart::ChartView::seriesDragged,
            m_program,
            &Domain::Program::applyManualMainRegression);
}

void MainWindow::getImage(QLabel* label, ChartType chart)
{
    QString imgPath = QFileDialog::getOpenFileName(
        this, tr("Выберите файл"),
        m_reportSaver->directory().absolutePath(),
        tr("Изображения (*.jpg *.png *.bmp)")
        );

    if (imgPath.isEmpty())
        return;

    QImage img(imgPath);

    if (img.isNull())
        return;

    QImage scaled = img.scaled(
        1000, 430,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
        );

    m_chartImageStorage.set(chart, scaled);

    if (label)
        label->setPixmap(QPixmap::fromImage(scaled));
}

void MainWindow::initClicked()
{
    m_isInitialized = false;
    setTaskControlsEnabled(false);
    ui->doubleSpinBox_task->setValue(4.0);

    ui->statusbar->showMessage(tr("Инициализация устройства..."));

    QVector<bool> states = {
        ui->pushButton_DO0->isChecked(),
        ui->pushButton_DO1->isChecked(),
        ui->pushButton_DO2->isChecked(),
        ui->pushButton_DO3->isChecked()
    };

    emit doInitStatesSelected(states);
    emit initialized();
    emit profileChanged(m_deviceProfile);
}

void MainWindow::restoreSeries(ChartType chart, const SeriesVisibilityBackup& b)
{
    auto* ch = m_chartManager->chart(chart);
    if (!ch) return;

    if (chart == ChartType::Task && b.visible.size() == 3) {
        ch->visible(2, b.visible[0]);
        ch->visible(3, b.visible[1]);
        ch->visible(4, b.visible[2]);
    }

    if (chart == ChartType::Pressure && !b.visible.isEmpty()) {
        ch->visible(1, b.visible.value(0, false));
        ch->visible(2, b.visible.value(1, false));
    }
}

void MainWindow::saveChart(ChartType chart)
{
    auto* chartView = m_chartManager->chart(chart);

    if (!chartView)
        return;

    const QImage image = m_chartImages->captureChart(chart);

    if (image.isNull())
        return;

    m_chartImageStorage.set(chart, image);

    if (auto* label = previewLabelForChart(chart)) {
        label->setPixmap(QPixmap::fromImage(image));
    }

    if (m_reportSaver) {
        m_reportSaver->saveChartSnapshot(chart, image, chartView);
    }
}

QLabel* MainWindow::previewLabelForChart(ChartType chart) const
{
    switch (chart) {
    case ChartType::Task:
        return ui->label_imageChartTask;
    case ChartType::Pressure:
        return ui->label_imageChartPressure;
    case ChartType::Friction:
        return ui->label_imageChartFriction;
    default:
        return nullptr;
    }
}

static quint32 timeTextToMs(const QString& text)
{
    const QTime time = QTime::fromString(text.trimmed(), "mm:ss.zzz");

    if (!time.isValid())
        return 0;

    return static_cast<quint32>(
        QTime(0, 0).msecsTo(time)
        );
}

void MainWindow::collectReportOverrides()
{
    // MainTestRecord
    if (m_telemetry.testMain) {
        NumberUtils::readDouble(ui->lineEdit_resultsTable_frictionForceValue,
                                m_telemetry.testMain->frictionForce);

        NumberUtils::readDouble(ui->lineEdit_resultsTable_frictionPercentValue,
                                m_telemetry.testMain->frictionPercent);

        NumberUtils::readDouble(ui->lineEdit_resultsTable_dynamicErrorReal,
                                m_telemetry.testMain->dynamicErrorReal);

        NumberUtils::readRange(ui->lineEdit_resultsTable_rangePressure,
                               m_telemetry.testMain->lowLimitPressure,
                               m_telemetry.testMain->highLimitPressure);

        NumberUtils::readRange(ui->lineEdit_resultsTable_driveRangeReal,
                               m_telemetry.testMain->springLow,
                               m_telemetry.testMain->springHigh);
    }

    // Stroke
    NumberUtils::readDouble(ui->lineEdit_resultsTable_strokeReal,
               m_telemetry.valveStrokeRecord.real);

    // Stroke test
    if (m_telemetry.testStroke) {
        m_telemetry.testStroke->forwardTimeMs =
            timeTextToMs(ui->lineEdit_resultsTable_strokeTest_forwardTime->text());

        m_telemetry.testStroke->backwardTimeMs =
            timeTextToMs(ui->lineEdit_resultsTable_strokeTest_backwardTime->text());
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

void MainWindow::editTestComment(const QString& testKey,
                                 const QString& description)
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Комментарий:"));

    auto* layout = new QVBoxLayout(&dialog);

    auto* descriptionLabel = new QLabel(description, &dialog);
    descriptionLabel->setWordWrap(true);

    auto* textEdit = new QTextEdit(&dialog);
    textEdit->setPlainText(m_testComments.value(testKey));
    textEdit->setMinimumSize(520, 260);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel,
        &dialog
        );

    layout->addWidget(descriptionLabel);
    layout->addWidget(textEdit);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString comment = textEdit->toPlainText().trimmed();

    if (comment.isEmpty()) {
        m_testComments.remove(testKey);
    } else {
        m_testComments[testKey] = comment;
    }

    saveCommentsPdfIfNeeded();
}

bool MainWindow::hasAnyTestComments() const
{
    for (auto it = m_testComments.constBegin(); it != m_testComments.constEnd(); ++it) {
        if (!it.value().trimmed().isEmpty())
            return true;
    }

    return false;
}

QString MainWindow::commentsPdfPath() const
{
    if (!m_reportSaver)
        return QString();

    return m_reportSaver->directory().filePath(tr("Комментарии.pdf"));
}

void MainWindow::saveCommentsPdfIfNeeded()
{
    if (!m_reportSaver)
        return;

    if (!m_reportSaver->ensureDirectory())
        return;

    const QString path = commentsPdfPath();

    if (path.isEmpty())
        return;

    if (!hasAnyTestComments()) {
        QFile::remove(path);
        return;
    }

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    writer.setPageMargins(
        QMarginsF(15, 15, 15, 15),
        QPageLayout::Millimeter
        );

    QTextDocument document;
    document.setHtml(buildCommentsHtml());
    document.print(&writer);
}

QString MainWindow::buildCommentsHtml() const
{
    auto block = [this](const QString& key,
                        const QString& title) -> QString
    {
        const QString comment = m_testComments.value(key).trimmed();

        if (comment.isEmpty())
            return QString();

        return QString(R"(
            <div class="card">
                <h2>%1</h2>

                <div class="comment-title">Комментарий</div>
                <div class="comment-text">%2</div>
            </div>
        )").arg(title.toHtmlEscaped(),
                 comment.toHtmlEscaped().replace(QStringLiteral("\n"),
                                                 QStringLiteral("<br>")));
    };

    QString html;

    html += QStringLiteral(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">

<style>
body {
    font-family: Arial, sans-serif;
    color: #222222;
}

h1 {
    font-size: 22pt;
    margin-bottom: 4px;
}

.subtitle {
    font-size: 9pt;
    color: #666666;
    margin-bottom: 20px;
}

.card {
    border: 1px solid #d0d0d0;
    padding: 12px;
    margin-bottom: 14px;
}

.card h2 {
    font-size: 14pt;
    margin: 0 0 4px 0;
}

.description {
    font-size: 9pt;
    color: #666666;
    margin-bottom: 10px;
}

.comment-block {
    border-left: 4px solid #fd7d13;
    background-color: #f6f6f6;
    padding: 8px 10px;
}

.comment-title {
    font-size: 9pt;
    font-weight: bold;
    color: #444444;
    margin-bottom: 4px;
}

.comment-text {
    font-size: 10pt;
    line-height: 1.4;
}
</style>
</head>

<body>
)");

    html += QStringLiteral("<h1>Комментарии к испытаниям</h1>");

    html += QStringLiteral("<div class=\"subtitle\">Сформировано: %1</div>")
                .arg(QDateTime::currentDateTime()
                         .toString(QStringLiteral("dd.MM.yyyy HH:mm"))
                         .toHtmlEscaped());

    html += block(
        QStringLiteral("mainTest"),
        tr("Основной тест")
        );

    html += QStringLiteral(R"(
</body>
</html>
)");

    return html;
}

void MainWindow::generateReportClicked()
{
    collectReportOverrides();

    ObjectInfo objectInfo = m_registry->objectInfo();
    ValveInfo valveInfo = m_registry->valveInfo();
    OtherParameters otherParameters = m_registry->otherParameters();

    collectRegistryOverrides(objectInfo, valveInfo, otherParameters);

    auto reportBuilder = Report::BuilderFactory::create(m_deviceProfile);

    if (!reportBuilder) {
        qDebug("Не выбран корректный паттерн отчёта!");
        return;
    }

    Report::ReportData report;
    reportBuilder->build(report,
                         m_telemetry,
                         m_registry->objectInfo(),
                         m_registry->valveInfo(),
                         m_registry->otherParameters(),
                         m_chartImageStorage);

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
    valveWindow.setProfile(m_deviceProfile);

    if (valveWindow.exec() == QDialog::Accepted) {
        setRegistry(m_registry);
    }

    this->show();
}

void MainWindow::generateFakePressureData()
{
    auto* chart = m_chartManager->chart(ChartType::Pressure);
    if (!chart) return;

    chart->clear();

    // Параметры реального теста:
    //   - kSteps ступеней позиции (0 → 100 мм и обратно)
    //   - kSamplesPerStep семплов на каждой ступени (клуббинг точек)
    //   - Трение = горизонтальный сдвиг между прямым и обратным ходом
    constexpr int    kSteps          = 12;
    constexpr int    kSamplesPerStep = 5;
    constexpr double kPosMin         = 0.0;   // мм
    constexpr double kPosMax         = 80.0;  // мм
    constexpr double kPressMin       = 2.0;   // бар
    constexpr double kPressMax       = 6.0;   // бар
    // Трение: прямой ход — давление чуть выше, обратный — чуть ниже
    constexpr double kFrictionBar    = 0.3;   // бар (горизонтальный сдвиг)

    auto* rng = QRandomGenerator::global();
    // Небольшой шум на давлении (±0.08 bar) и позиции (±0.3 мм)
    auto nP = [&]() { return (rng->generateDouble() - 0.5) * 0.16; };
    auto nY = [&]() { return (rng->generateDouble() - 0.5) * 0.6;  };

    // Линейная зависимость: position = k * pressure + b
    // Из условий: pos(pressMin) = posMin, pos(pressMax) = posMax
    const double kTrue = (kPosMax - kPosMin) / (kPressMax - kPressMin);
    const double bFwd  = kPosMin - kTrue * kPressMin + kFrictionBar * kTrue / 2.0;
    const double bBwd  = kPosMin - kTrue * kPressMin - kFrictionBar * kTrue / 2.0;

    QVector<QPointF> fwdScatter, bwdScatter;

    // --- Прямой ход: position идёт ступеньками 0 → kPosMax ---
    for (int step = 0; step <= kSteps; ++step) {
        const double pos = kPosMin + (kPosMax - kPosMin) * step / kSteps;
        // "правильное" давление для этой позиции (прямой ход чуть правее)
        const double pressBase = (pos - bFwd) / kTrue;
        for (int s = 0; s < kSamplesPerStep; ++s) {
            const double p   = pressBase + nP();
            const double y   = pos       + nY();
            chart->addPoint(0, p, y);
            fwdScatter.append({p, y});
        }
    }

    // --- Обратный ход: position идёт ступеньками kPosMax → 0 ---
    for (int step = kSteps; step >= 0; --step) {
        const double pos = kPosMin + (kPosMax - kPosMin) * step / kSteps;
        // обратный ход — давление сдвинуто влево (меньше при той же позиции)
        const double pressBase = (pos - bBwd) / kTrue;
        for (int s = 0; s < kSamplesPerStep; ++s) {
            const double p   = pressBase + nP();
            const double y   = pos       + nY();
            chart->addPoint(0, p, y);
            bwdScatter.append({p, y});
        }
    }

    // --- Линейная регрессия по scatter-точкам ---
    auto linReg = [](const QVector<QPointF>& pts, double& k, double& b) {
        double sx = 0, sy = 0, sxy = 0, sx2 = 0;
        const int n = pts.size();
        for (const auto& pt : pts) {
            sx  += pt.x(); sy  += pt.y();
            sxy += pt.x() * pt.y();
            sx2 += pt.x() * pt.x();
        }
        const double d = n * sx2 - sx * sx;
        if (qFuzzyIsNull(d)) { k = 0; b = 0; return; }
        k = (n * sxy - sx * sy) / d;
        b = (sy - k * sx) / n;
    };

    double k1, b1, k2, b2;
    linReg(fwdScatter, k1, b1);
    linReg(bwdScatter, k2, b2);

    // --- Добавляем линии регрессии как полилинии с несколькими точками управления ---
    constexpr int kRegPts = 5;
    for (int i = 0; i < kRegPts; ++i) {
        const double t = double(i) / (kRegPts - 1);
        const double p = kPressMin + (kPressMax - kPressMin) * t;
        chart->addPoint(1, p, k1 * p + b1);
        chart->addPoint(2, p, k2 * p + b2);
    }

    chart->visible(1, true);
    chart->visible(2, true);
    ui->checkBox_regression->setEnabled(true);
    ui->checkBox_regression->setCheckState(Qt::Checked);

    // --- Инициализируем контекст в Program для пересчёта при перетаскивании ---
    Domain::Tests::Main::Result fakeResult;
    auto& ctx        = fakeResult.regressionCtx;
    ctx.k1 = k1; ctx.b1 = b1;
    ctx.k2 = k2; ctx.b2 = b2;
    ctx.limMinX = kPressMin; ctx.limMaxX = kPressMax;
    ctx.limMinY = kPosMin;   ctx.limMaxY = kPosMax;
    ctx.valid = true;
    m_program->onMainResultReceived(fakeResult);
}
