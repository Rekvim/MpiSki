#include "Program.h"

// #include "./Src/Tests/CyclicTestPositioner.h"
#include "./Src/Tests/CyclicTestSolenoid.h"
#include "./Src/Tests/StepTest.h"
#include "./Src/Tests/StrokeTest.h"
#include "./Src/Tests/MainTest.h"


Program::Program(QObject *parent)
    : QObject{parent}
{
    m_timerSensors = new QTimer(this);
    m_timerSensors->setInterval(200);
    connect(m_timerSensors, &QTimer::timeout, this, &Program::UpdateSensors);
    m_testing = false;
    m_dacEventloop = new QEventLoop(this);



    m_timerDI = new QTimer(this);
    m_timerDI->setInterval(1000);
    connect(m_timerDI, &QTimer::timeout, this, [&]() {
        quint8 DI = m_mpi.GetDIStatus();
        emit SetCheckboxDIChecked(DI);
    });

    connect(m_timerSensors, &QTimer::timeout,
            this, &Program::UpdateCharts_CyclicSolenoid);
}

void Program::SetRegistry(Registry *registry)
{
    m_registry = registry;
}

void Program::SetDAC(quint16 dac, quint32 sleep_ms, bool wait_for_stop, bool wait_for_start)
{
    m_stopSetDac = false;

    if (m_mpi.SensorCount() == 0) {
        emit ReleaseBlock();
        return;
    }

    m_mpi.SetDAC_Raw(dac);

    if (wait_for_start) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->GetRawValue());
            if (qAbs(lineSensor.first() - lineSensor.last()) > 10) {
                timer.stop();
                m_dacEventloop->quit();
            }
            if (lineSensor.size() > 50) {
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) {
        emit ReleaseBlock();
        return;
    }

    if (sleep_ms > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout, m_dacEventloop, &QEventLoop::quit);
        timer.start(sleep_ms);
        m_dacEventloop->exec();
        timer.stop();
    }

    if (m_stopSetDac) {
        emit ReleaseBlock();
        return;
    }

    if (wait_for_stop) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> lineSensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            lineSensor.push_back(m_mpi[0]->GetRawValue());
            if (lineSensor.size() == 50) {
                if (qAbs(lineSensor.first() - lineSensor.last()) < 10) {
                    timer.stop();
                    m_dacEventloop->quit();
                }
                lineSensor.pop_front();
            }
        });

        timer.start();
        m_dacEventloop->exec();
        timer.stop();
    }

    emit ReleaseBlock();
}

void Program::SetTimeStart()
{
    m_startTime = QDateTime::currentMSecsSinceEpoch();
}

void Program::StrokeTestResults(quint64 forwardTime, quint64 backwardTime)
{
    QString forwardText = QTime(0, 0).addMSecs(forwardTime).toString("mm:ss.zzz");
    QString backwardText = QTime(0, 0).addMSecs(backwardTime).toString("mm:ss.zzz");
    emit SetText(TextObjects::Label_strokeTest_forwardTime, forwardText);
    emit SetText(TextObjects::LineEdit_strokeTest_forwardTime, forwardText);
    emit SetText(TextObjects::Label_strokeTest_backwardTime, backwardText);
    emit SetText(TextObjects::LineEdit_strokeTest_backwardTime, backwardText);
}

void Program::AddRegression(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;
    for (QPointF point : points) {
        chartPoints.push_back({1, point.x(), point.y()});
    }
    emit AddPoints(Charts::Pressure, chartPoints);

    //emit SetVisible(Charts::Pressure, 1, true);
    emit SetRegressionEnable(true);
}

void Program::AddFriction(const QVector<QPointF> &points)
{
    QVector<Point> chartPoints;

    ValveInfo *valveInfo = m_registry->GetValveInfo();

    qreal k = 5 * M_PI * valveInfo->driveDiameter * valveInfo->driveDiameter / 4;

    for (QPointF point : points) {
        chartPoints.push_back({0, point.x(), point.y() * k});
    }
    emit AddPoints(Charts::Friction, chartPoints);
}

void Program::UpdateSensors()
{
    for (quint8 i = 0; i < m_mpi.SensorCount(); ++i) {
        switch (i) {
        case 0:
            if (m_blockCts.moving) {
                emit SetText(TextObjects::LineEdit_linearSensor, m_mpi[i]->GetFormatedValue());
                emit SetText(TextObjects::LineEdit_linearSensorPercent, m_mpi[i]->GetPersentFormated());
                break;
            }
            break;
        case 1:
            if (m_blockCts.pressure_1) {
                emit SetText(TextObjects::LineEdit_pressureSensor_1, m_mpi[i]->GetFormatedValue());
                break;
            }
            break;
        case 2:
            if (m_blockCts.pressure_2) {
                emit SetText(TextObjects::LineEdit_pressureSensor_2, m_mpi[i]->GetFormatedValue());
                break;
            }
            break;
        case 3:
            if (m_blockCts.pressure_3) {
                emit SetText(TextObjects::LineEdit_pressureSensor_3, m_mpi[i]->GetFormatedValue());
                break;
            }
            break;
        }
    }

    {
        Sensor *feedbackSensor = m_mpi.GetDAC();
        QString fbValue = feedbackSensor->GetFormatedValue();
        emit SetText(TextObjects::LineEdit_feedback_4_20mA, fbValue);
    }

    if (m_testing)
        emit SetTask(m_mpi.GetDAC()->GetValue());

    QVector<Point> points;
    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_initTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::Trend, points);
}

void Program::UpdateCharts_maintest()
{
    QVector<Point> points;
    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    qreal task = m_mpi[0]->GetValueFromPercent(percent);
    qreal X = m_mpi.GetDAC()->GetValue();
    points.push_back({0, X, task});

    for (quint8 i = 0; i < m_mpi.SensorCount(); ++i) {
        points.push_back({static_cast<quint8>(i + 1), X, m_mpi[i]->GetValue()});
    }

    emit AddPoints(Charts::Task, points);

    points.clear();
    points.push_back({0, m_mpi[1]->GetValue(), m_mpi[0]->GetValue()});

    emit AddPoints(Charts::Pressure, points);
}

void Program::UpdateCharts_optiontest(Charts chart)
{
    QVector<Point> points;

    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(chart, points);
}

void Program::MainTestResults(MainTest::TestResults results)
{
    ValveInfo *valveInfo = m_registry->GetValveInfo();

    qreal k = 5 * M_PI * valveInfo->driveDiameter * valveInfo->driveDiameter / 4;

    emit SetText(TextObjects::Label_pressureDiff,
                 QString::asprintf("%.3f bar", results.pressureDiff));
    emit SetText(TextObjects::Label_friction,
                 QString::asprintf("%.3f H", results.pressureDiff * k));
    emit SetText(TextObjects::Label_din_error_mean,
                 QString::asprintf("%.3f mA", results.dinErrorMean));
    emit SetText(TextObjects::Label_din_error_max,
                 QString::asprintf("%.3f mA", results.dinErrorMax));
    emit SetText(TextObjects::Label_din_error_mean_percent,
                 QString::asprintf("%.2f %%", results.dinErrorMean / 0.16));
    emit SetText(TextObjects::Label_din_error_max_percent,
                 QString::asprintf("%.2f %%", results.dinErrorMax / 0.16));
    emit SetText(TextObjects::Label_frictionPercent,
                 QString::asprintf("%.2f %%", results.friction));
    emit SetText(TextObjects::Label_low_limit, QString::asprintf("%.2f bar", results.lowLimit));
    emit SetText(TextObjects::Label_high_limit, QString::asprintf("%.2f bar", results.highLimit));

    emit SetText(TextObjects::LineEdit_dinamicError,
                 QString::asprintf("%.2f", results.dinErrorMean / 0.16));
    emit SetText(TextObjects::LineEdit_rangePressure,
                 QString::asprintf("%.2f - %.2f", results.lowLimit, results.highLimit));
    emit SetText(TextObjects::LineEdit_range,
                 QString::asprintf("%.2f - %.2f", results.springLow, results.springHigh));

    emit SetText(TextObjects::LineEdit_friction,
                 QString::asprintf("%.3f", results.pressureDiff * k));
    emit SetText(TextObjects::LineEdit_frictionPercent,
                 QString::asprintf("%.2f", results.friction));
}

void Program::StepTestResults(QVector<StepTest::TestResult> results, quint32 T_value)
{
    emit SetStepResults(results, T_value);
}

void Program::GetPoints_maintest(QVector<QVector<QPointF> > &points)
{
    emit GetPoints(points, Charts::Task);
}

void Program::GetPoints_steptest(QVector<QVector<QPointF> > &points)
{
    emit GetPoints(points, Charts::Step);
}

void Program::EndTest()
{
    m_testing = false;
    emit EnableSetTask(true);
    emit SetButtonInitEnabled(true);
    SetDAC_int(0);
    emit StopTest();
    emit SetTask(m_mpi.GetDAC()->GetValue());
    m_timerSensors->stop();
}

void Program::SetDAC_real(qreal value)
{
    m_mpi.SetDAC_Real(value);
}

void Program::SetDAC_int(quint16 value)
{
    qreal mA = 4.0 + (16.0 * value / 100.0);

    quint16 raw = m_mpi.GetDAC()->GetRawFromValue(mA);


    m_mpi.SetDAC_Raw(raw);


    emit ReleaseBlock();
}
void Program::SetBlockCTS(const SelectTests::BlockCTS &blockCTS)
{
    m_blockCts = blockCTS;
}
void Program::button_init()
{
    m_timerSensors->stop();
    m_timerDI->stop();

    emit SetButtonInitEnabled(false);
    emit SetSensorNumber(0);

    // emit SetGroupDOVisible(false);

    emit SetText(TextObjects::Label_status, "");
    emit SetText(TextObjects::Label_init, "");
    emit SetText(TextObjects::Label_sensors, "");
    emit SetText(TextObjects::Label_startValue, "");
    emit SetText(TextObjects::Label_endValue, "");

    if (!m_mpi.Connect()) {
        emit SetText(TextObjects::Label_status, "Ошибка подключения");
        emit SetTextColor(TextObjects::Label_status, Qt::red);
        emit SetButtonInitEnabled(true);
        return;
    }

    emit SetText(TextObjects::Label_status, "Успешное подключение к порту " + m_mpi.PortName());
    emit SetTextColor(TextObjects::Label_status, Qt::darkGreen);

    if (!m_mpi.Initialize()) {
        emit SetText(TextObjects::Label_init, "Ошибка инициализации");
        emit SetTextColor(TextObjects::Label_status, Qt::red);
        emit SetButtonInitEnabled(true);
        return;
    }

    if ((m_mpi.Version() & 0x40) != 0) {
       // emit SetGroupDOVisible(true);
       emit SetButtonsDOChecked(m_mpi.GetDOStatus());
       m_timerDI->start();
    }

    emit SetText(TextObjects::Label_init, "Успешная инициализация");
    emit SetTextColor(TextObjects::Label_init, Qt::darkGreen);

    m_isInitialized = true;

    switch (m_mpi.SensorCount()) { // update
    case 0:
        emit SetText(TextObjects::Label_sensors, "Дачики не обнаружены");
        emit SetTextColor(TextObjects::Label_sensors, Qt::red);
        // emit SetButtonInitEnabled(true);
        // return;
    case 1:
        emit SetText(TextObjects::Label_sensors, "Обнаружен 1 датчик");
        emit SetTextColor(TextObjects::Label_sensors, Qt::darkYellow);
        break;
    default:
        emit SetText(TextObjects::Label_sensors,
                     "Обнаружено " + QString::number(m_mpi.SensorCount()) + " датчика");
        emit SetTextColor(TextObjects::Label_sensors, Qt::darkGreen);
    }

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    bool normalClosed = (valveInfo->safePosition == 0);

    emit SetText(TextObjects::Label_startValue, "Измерение");
    emit SetTextColor(TextObjects::Label_startValue, Qt::darkYellow);

    SetDAC(0, 10000, true);

    QTimer timer(this);
    connect(&timer, &QTimer::timeout, this, [&] {
        if (!m_waitForButton || m_stopSetDac) {
            m_dacEventloop->quit();
        }
    });

    timer.start(50);
    m_dacEventloop->exec();
    timer.stop();

    if (normalClosed)
        m_mpi[0]->SetMin();
    else
        m_mpi[0]->SetMax();

    emit SetText(TextObjects::Label_startValue, m_mpi[0]->GetFormatedValue());
    emit SetTextColor(TextObjects::Label_startValue, Qt::darkGreen);

    emit SetText(TextObjects::Label_endValue, "Измерение");
    emit SetTextColor(TextObjects::Label_endValue, Qt::darkYellow);

    SetDAC(0xFFFF, 10000, true);

    timer.start(50);
    m_dacEventloop->exec();
    timer.stop();

    if (normalClosed)
        m_mpi[0]->SetMax();
    else
        m_mpi[0]->SetMin();

    emit SetText(TextObjects::Label_endValue, m_mpi[0]->GetFormatedValue());
    emit SetTextColor(TextObjects::Label_endValue, Qt::darkGreen);

    qreal correctCoefficient = 1;
    if (valveInfo->strokeMovement != 0) {
        correctCoefficient = qRadiansToDegrees(2 / valveInfo->diameterPulley);
        m_mpi[0]->SetUnit("°");
    }
    m_mpi[0]->CorrectCoefficients(correctCoefficient);

    if (normalClosed) {
        emit SetText(TextObjects::Label_range, m_mpi[0]->GetFormatedValue());
        emit SetText(TextObjects::LineEdit_stroke, QString::asprintf("%.2f", m_mpi[0]->GetValue()));
        SetDAC(0);
    } else {
        SetDAC(0, 10000, true);
        emit SetText(TextObjects::Label_range, m_mpi[0]->GetFormatedValue());
        emit SetText(TextObjects::LineEdit_stroke, QString::asprintf("%.2f", m_mpi[0]->GetValue()));
    }

    emit SetTask(m_mpi.GetDAC()->GetValue());

    emit ClearPoints(Charts::Trend);
    m_initTime = QDateTime::currentMSecsSinceEpoch();

    emit SetSensorNumber(m_mpi.SensorCount());
    emit SetButtonInitEnabled(true);
    m_timerSensors->start();
}

bool Program::isInitialized() const {
    return m_isInitialized;
}

void Program::MainTestStart()
{
    MainTestSettings::TestParameters parameters;
    emit GetMainTestParameters(parameters);

    if (parameters.delay == 0) {
        emit StopTest();
        return;
    }

    parameters.dac_min = qMax(m_mpi.GetDAC()->GetRawFromValue(parameters.signal_min),
                              m_mpi.GetDac_Min());
    parameters.dac_max = qMin(m_mpi.GetDAC()->GetRawFromValue(parameters.signal_max),
                              m_mpi.GetDac_Max());

    emit SetButtonInitEnabled(false);

    // MainTest *main_test = parameters.is_cyclic ? new CyclicTestPositioner : new MainTest;

    MainTest *main_test = new MainTest;

    main_test->SetParameters(parameters);

    QThread *threadTest = new QThread(this);
    main_test->moveToThread(threadTest);

    // if (parameters.is_cyclic) {
    //     connect(dynamic_cast<CyclicTestPositioner *>(main_test),
    //             &CyclicTestPositioner::UpdateCyclicTred,
    //             this,
    //             &Program::UpdateCharts_CyclicTred);
    //     connect(dynamic_cast<CyclicTestPositioner *>(main_test),
    //             &CyclicTestPositioner::SetStartTime,
    //             this,
    //             &Program::SetTimeStart);
    // }

    connect(threadTest, &QThread::started, main_test, &MainTest::Process);
    connect(main_test, &MainTest::EndTest, threadTest, &QThread::quit);

    connect(this, &Program::StopTest, main_test, &MainTest::Stop);
    connect(threadTest, &QThread::finished, threadTest, &QThread::deleteLater);
    connect(threadTest, &QThread::finished, main_test, &MainTest::deleteLater);

    connect(main_test, &MainTest::EndTest, this, &Program::EndTest);

    connect(main_test, &MainTest::UpdateGraph, this, &Program::UpdateCharts_maintest);
    connect(main_test, &MainTest::SetDAC, this, &Program::SetDAC);

    connect(main_test, &MainTest::DublSeries, this, [&] { emit DublSeries(); });
    connect(main_test,
            &MainTest::GetPoints,
            this,
            &Program::GetPoints_maintest,
            Qt::BlockingQueuedConnection);

    connect(main_test, &MainTest::AddRegression, this, &Program::AddRegression);
    connect(main_test, &MainTest::AddFriction, this, &Program::AddFriction);

    connect(this, &Program::ReleaseBlock, main_test, &MainTest::ReleaseBlock);
    connect(main_test, &MainTest::Results, this, &Program::MainTestResults);

    connect(main_test, &MainTest::ShowDots, this, [&](bool visible) { emit ShowDots(visible); });

    connect(main_test, &MainTest::ClearGraph, this, [&] {
        emit ClearPoints(Charts::Task);
        emit ClearPoints(Charts::Pressure);
        emit ClearPoints(Charts::Friction);
        emit SetRegressionEnable(false);
    });

    // emit ClearPoints(Charts::Cyclic);

    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}

void Program::StrokeTestStart()
{
    emit SetButtonInitEnabled(false);
    emit ClearPoints(Charts::Stroke);

    StrokeTest *strokeTest = new StrokeTest;
    QThread *threadTest = new QThread(this);
    strokeTest->moveToThread(threadTest);

    connect(threadTest, &QThread::started, strokeTest, &StrokeTest::Process);
    connect(strokeTest, &StrokeTest::EndTest, threadTest, &QThread::quit);

    connect(this, &Program::StopTest, strokeTest, &StrokeTest::Stop);
    connect(threadTest, &QThread::finished, threadTest, &QThread::deleteLater);
    connect(threadTest, &QThread::finished, strokeTest, &StrokeTest::deleteLater);

    connect(this, &Program::ReleaseBlock, strokeTest, &MainTest::ReleaseBlock);

    connect(strokeTest, &StrokeTest::EndTest, this, &Program::EndTest);

    connect(strokeTest, &StrokeTest::UpdateGraph, this, &Program::UpdateCharts_stroketest);
    connect(strokeTest, &StrokeTest::SetDAC, this, &Program::SetDAC);
    connect(strokeTest, &StrokeTest::SetStartTime, this, &Program::SetTimeStart);
    connect(strokeTest, &StrokeTest::Results, this, &Program::StrokeTestResults);
    m_testing = true;
    emit EnableSetTask(false);
    threadTest->start();
}

void Program::UpdateCharts_stroketest()
{
    QVector<Point> points;

    qreal percent = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valveInfo = m_registry->GetValveInfo();
    if (valveInfo->safePosition != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), m_mpi[0]->GetPersent()});

    emit AddPoints(Charts::Stroke, points);
}

void Program::UpdateCharts_CyclicSolenoid()
{
    if (!m_testing)
        return;

    qreal task = ((m_mpi.GetDAC()->GetValue() - 4) / 16) * 100;
    task = qBound<qreal>(0.0, task, 100.0);


    qreal measured = m_mpi[0]->GetPersent();


    quint64 t = QDateTime::currentMSecsSinceEpoch() - m_startTime;

    QVector<Point> pts;

    pts.push_back({0, qreal(t), task});
    pts.push_back({1, qreal(t), measured});

    emit AddPoints(Charts::CyclicSolenoid, pts);
}

void Program::CyclicSolenoidTestStart(const CyclicTestSettings::TestParameters &p)
{
    if (p.sequence.isEmpty()
        || p.delay_sec <= 0
        || p.hold_time_sec < 0
        || p.num_cycles <= 0)
    {
        emit StopTest();
        return;
    }

    auto *sol = new CyclicTestSolenoid;
    sol->SetParameters(p.sequence,
                       p.delay_sec,
                       p.hold_time_sec,
                       p.num_cycles);

    QThread *thr = new QThread(this);
    sol->moveToThread(thr);


    connect(thr, &QThread::started, sol, &CyclicTestSolenoid::Process);


    connect(sol, &CyclicTestSolenoid::RequestSensorRawValue,
            this, [&](quint16 &value) {
                value = m_mpi[0]->GetRawValue();
            },
            Qt::BlockingQueuedConnection);

    // connect(sol, &CyclicTestSolenoid::UpdateCyclicTred,
    //         this, &Program::UpdateCharts_CyclicSolenoid);

    connect(sol, &CyclicTestSolenoid::EndTest, thr, &QThread::quit);
    connect(sol, &CyclicTestSolenoid::EndTest, this, &Program::EndTest);


    connect(thr, &QThread::finished, sol, &QObject::deleteLater);
    connect(thr, &QThread::finished, thr, &QObject::deleteLater);

    connect(sol, &CyclicTestSolenoid::ClearGraph, this,
            [&]{ emit ClearPoints(Charts::CyclicSolenoid); });

    // в Program::CyclicSolenoidTestStart()
    connect(sol, &CyclicTestSolenoid::TaskPoint, this,
            [&](qint64 t, int pct){
                QVector<Point> pts;
                pts.push_back({0, qreal(t), qreal(pct)});
                emit AddPoints(Charts::CyclicSolenoid, pts);
            });


    connect(sol, &CyclicTestSolenoid::SetStartTime,
            this, &Program::SetTimeStart);

    connect(sol, &CyclicTestSolenoid::SetSolenoidResults,
            this, &Program::SolenoidResults);

    connect(sol, &CyclicTestSolenoid::SetDAC,
            this, &Program::SetDAC_int);

    emit ClearPoints(Charts::CyclicSolenoid);
    m_testing = true;
    emit EnableSetTask(false);


    thr->start();

    m_timerSensors->start();
}

void Program::SolenoidResults(double forwardSec,
                              double backwardSec,
                              quint16 cycles,
                              double rangePercent,
                              double totalTimeSec)
{
    emit SetSolenoidResults(forwardSec, backwardSec, cycles, rangePercent, totalTimeSec);
}

void Program::StartOptionalTest(quint8 testNum)
{
    OptionTest::Task task;
    OptionTest *optionalTest;

    switch (testNum) {
    case 0: {
        optionalTest = new OptionTest;

        OtherTestSettings::TestParameters parameters;
        emit GetResponseTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valveInfo = m_registry->GetValveInfo();

        bool normalOpen = (valveInfo->safePosition != 0);

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(4.0));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            for (quint8 i = 0; i < 2; i++) {
                qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
                qreal dacValue = m_mpi.GetDAC()->GetRawFromValue(current);
                task.value.push_back(dacValue);

                for (auto it_s = parameters.steps.begin(); it_s < parameters.steps.end(); ++it_s) {
                    current += (16 * *it_s / 100) * (i == 0 ? 1 : -1) * (normalOpen ? -1 : 1);
                    dacValue = m_mpi.GetDAC()->GetRawFromValue(current);
                    task.value.push_back(dacValue);
                }
            }
        }

        optionalTest->SetTask(task);

        connect(optionalTest, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optiontest(Charts::Response);
        });

        emit ClearPoints(Charts::Response);

        break;
    }
    case 1: {
        optionalTest = new OptionTest;
        OtherTestSettings::TestParameters parameters;
        emit GetResolutionTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valveInfo = m_registry->GetValveInfo();

        bool normalOpen = (valveInfo->safePosition != 0);

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(4.0));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            qreal current = 16.0 * (normalOpen ? 100 - *it : *it) / 100 + 4.0;
            qreal dac_value = m_mpi.GetDAC()->GetRawFromValue(current);

            for (auto it_s = parameters.steps.begin(); it_s < parameters.steps.end(); ++it_s) {
                task.value.push_back(dac_value);
                current = (16 * (normalOpen ? 100 - *it - *it_s : *it + *it_s) / 100 + 4.0);
                qreal dac_value_step = m_mpi.GetDAC()->GetRawFromValue(current);
                task.value.push_back(dac_value_step);
            }
        }

        optionalTest->SetTask(task);

        connect(optionalTest, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optiontest(Charts::Resolution);
        });

        emit ClearPoints(Charts::Resolution);

        break;
    }

    case 2: {
        optionalTest = new StepTest;
        StepTestSettings::TestParameters parameters;
        emit GetStepTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optionalTest;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;
        ValveInfo *valveInfo = m_registry->GetValveInfo();

        qreal startValue = 4.0;
        qreal endValue = 20.0;

        bool normal_open = (valveInfo->safePosition != 0);

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(startValue));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            qreal current = 16.0 * (normal_open ? 100 - *it : *it) / 100 + 4.0;
            qreal dac_value = m_mpi.GetDAC()->GetRawFromValue(current);
            task.value.push_back(dac_value);
        }

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(endValue));

        for (auto it = parameters.points.rbegin(); it != parameters.points.rend(); ++it) {
            qreal current = 16.0 * (normal_open ? 100 - *it : *it) / 100 + 4.0;
            qreal dac_value = m_mpi.GetDAC()->GetRawFromValue(current);
            task.value.push_back(dac_value);
        }

        task.value.push_back(m_mpi.GetDAC()->GetRawFromValue(startValue));

        optionalTest->SetTask(task);
        dynamic_cast<StepTest *>(optionalTest)->Set_T_value(parameters.test_value);

        connect(optionalTest, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optiontest(Charts::Step);
        });
        connect(dynamic_cast<StepTest *>(optionalTest),
                &StepTest::GetPoints,
                this,
                &Program::GetPoints_steptest,
                Qt::BlockingQueuedConnection);
        connect(dynamic_cast<StepTest *>(optionalTest),
                &StepTest::Results,
                this,
                &Program::StepTestResults);
        emit ClearPoints(Charts::Step);

        break;
    }
    default:
        emit StopTest();
        return;
    }

    QThread *threadTest = new QThread(this);
    optionalTest->moveToThread(threadTest);

    emit SetButtonInitEnabled(false);

    connect(threadTest, &QThread::started, optionalTest, &OptionTest::Process);
    connect(optionalTest, &OptionTest::EndTest, threadTest, &QThread::quit);

    connect(this, &Program::StopTest, optionalTest, &OptionTest::Stop);
    connect(threadTest, &QThread::finished, threadTest, &QThread::deleteLater);
    connect(threadTest, &QThread::finished, optionalTest, &OptionTest::deleteLater);

    connect(this, &Program::ReleaseBlock, optionalTest, &MainTest::ReleaseBlock);

    connect(optionalTest, &OptionTest::EndTest, this, &Program::EndTest);

    connect(optionalTest, &OptionTest::SetDAC, this, &Program::SetDAC);
    connect(optionalTest, &OptionTest::SetStartTime, this, &Program::SetTimeStart);

    m_testing = true;
    emit EnableSetTask(false);
    emit ClearPoints(Charts::CyclicSolenoid);
    threadTest->start();
}

void Program::TerminateTest()
{
    m_stopSetDac = true;
    m_dacEventloop->quit();
    emit StopTest();
}


void Program::button_open() {}

void Program::button_report() {}

void Program::button_pixmap1() {}

void Program::button_pixmap2() {}

void Program::button_pixmap3() {}

void Program::button_set_position()
{
    m_stopSetDac = true;
    m_dacEventloop->quit();
}

void Program::button_DO(quint8 DO_num, bool state)
{
    m_mpi.SetDiscreteOutput(DO_num, state);
}

void Program::checkbox_autoinit(int state)
{
    m_waitForButton = (state == 0);
}
