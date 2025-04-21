#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>

#include "Program.h"

Program::Program(QObject *parent)
    : QObject{parent}
{
    timer_sensors_ = new QTimer(this);
    timer_sensors_->setInterval(200);
    connect(timer_sensors_, &QTimer::timeout, this, &Program::UpdateSensors);
    testing_ = false;
    dac_eventloop_ = new QEventLoop(this);

    timer_DI_ = new QTimer(this);
    timer_DI_->setInterval(1000);
    connect(timer_DI_, &QTimer::timeout, this, [&]() {
        quint8 DI = mpi_.GetDIStatus();
        emit SetCheckboxDIChecked(DI);
    });
}

void Program::SetRegistry(Registry *registry)
{
    registry_ = registry;
}

void Program::SetDAC(quint16 dac, quint32 sleep_ms, bool wait_for_stop, bool wait_for_start)
{
    stop_set_dac_ = false;

    if (mpi_.SensorCount() == 0) {
        emit ReleaseBlock();
        return;
    }

    mpi_.SetDAC_Raw(dac);

    if (wait_for_start) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> line_sensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            line_sensor.push_back(mpi_[0]->GetRawValue());
            if (qAbs(line_sensor.first() - line_sensor.last()) > 10) {
                timer.stop();
                dac_eventloop_->quit();
            }
            if (line_sensor.size() > 50) {
                line_sensor.pop_front();
            }
        });

        timer.start();
        dac_eventloop_->exec();
        timer.stop();
    }

    if (stop_set_dac_) {
        emit ReleaseBlock();
        return;
    }

    if (sleep_ms > 20) {
        QTimer timer;
        connect(&timer, &QTimer::timeout, dac_eventloop_, &QEventLoop::quit);
        timer.start(sleep_ms);
        dac_eventloop_->exec();
        timer.stop();
    }

    if (stop_set_dac_) {
        emit ReleaseBlock();
        return;
    }

    if (wait_for_stop) {
        QTimer timer;
        timer.setInterval(50);

        QList<quint16> line_sensor;

        connect(&timer, &QTimer::timeout, this, [&]() {
            line_sensor.push_back(mpi_[0]->GetRawValue());
            if (line_sensor.size() == 50) {
                if (qAbs(line_sensor.first() - line_sensor.last()) < 10) {
                    timer.stop();
                    dac_eventloop_->quit();
                }
                line_sensor.pop_front();
            }
        });

        timer.start();
        dac_eventloop_->exec();
        timer.stop();
    }

    emit ReleaseBlock();
}

void Program::SetTimeStart()
{
    start_time_ = QDateTime::currentMSecsSinceEpoch();
}

void Program::StrokeTestResults(quint64 forward_time, quint64 backward_time)
{
    QString forward_text = QTime(0, 0).addMSecs(forward_time).toString("mm:ss.zzz");
    QString backward_text = QTime(0, 0).addMSecs(backward_time).toString("mm:ss.zzz");
    emit SetText(TextObjects::Label_forward, forward_text);
    emit SetText(TextObjects::LineEdit_forward, forward_text);
    emit SetText(TextObjects::Label_backward, backward_text);
    emit SetText(TextObjects::LineEdit_backward, backward_text);
}

void Program::AddRegression(const QVector<QPointF> &points)
{
    QVector<Point> chart_points;
    for (QPointF point : points) {
        chart_points.push_back({1, point.x(), point.y()});
    }
    emit AddPoints(Charts::Main_pressure, chart_points);

    //emit SetVisible(Charts::Main_pressure, 1, true);
    emit SetRegressionEnable(true);
}

void Program::AddFriction(const QVector<QPointF> &points)
{
    QVector<Point> chart_points;

    ValveInfo *valve_info = registry_->GetValveInfo();

    qreal k = 5 * M_PI * valve_info->diameter * valve_info->diameter / 4;

    for (QPointF point : points) {
        chart_points.push_back({0, point.x(), point.y() * k});
    }
    emit AddPoints(Charts::Main_friction, chart_points);
}

void Program::UpdateSensors()
{
    for (quint8 i = 0; i < mpi_.SensorCount(); ++i) {
        switch (i) {
        case 0:
            if (block_cts_.moving) {
                emit SetText(TextObjects::LineEdit_linear_sensor, mpi_[i]->GetFormatedValue());
                emit SetText(TextObjects::LineEdit_linear_sensor_percent, mpi_[i]->GetPersentFormated());
                break;
            }
        case 1:
            if (block_cts_.pressure_1) {
                emit SetText(TextObjects::LineEdit_pressure_sensor1, mpi_[i]->GetFormatedValue());
                break;
            }
        case 2:
            if (block_cts_.pressure_1) {
                emit SetText(TextObjects::LineEdit_pressure_sensor2, mpi_[i]->GetFormatedValue());
                break;
            }
            break;
        case 3:
            if (block_cts_.pressure_1) {
                emit SetText(TextObjects::LineEdit_pressure_sensor3, mpi_[i]->GetFormatedValue());
                break;
            }
            break;
        }
    }
    if (testing_)
        emit SetTask(mpi_.GetDAC()->GetValue());

    QVector<Point> points;
    qreal percent = ((mpi_.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valve_info = registry_->GetValveInfo();
    if (valve_info->normal_position != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - init_time_;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), mpi_[0]->GetPersent()});

    emit AddPoints(Charts::Trend, points);
}

void Program::UpdateCharts_maintest()
{
    QVector<Point> points;
    qreal percent = ((mpi_.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valve_info = registry_->GetValveInfo();
    if (valve_info->normal_position != 0) {
        percent = 100 - percent;
    }

    qreal task = mpi_[0]->GetValueFromPercent(percent);
    qreal X = mpi_.GetDAC()->GetValue();
    points.push_back({0, X, task});

    for (quint8 i = 0; i < mpi_.SensorCount(); ++i) {
        points.push_back({static_cast<quint8>(i + 1), X, mpi_[i]->GetValue()});
    }

    emit AddPoints(Charts::Main_task, points);

    points.clear();
    points.push_back({0, mpi_[1]->GetValue(), mpi_[0]->GetValue()});

    emit AddPoints(Charts::Main_pressure, points);
}

void Program::UpdateCharts_stroketest()
{
    QVector<Point> points;

    qreal percent = ((mpi_.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valve_info = registry_->GetValveInfo();
    if (valve_info->normal_position != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - start_time_;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), mpi_[0]->GetPersent()});

    emit AddPoints(Charts::Stroke, points);
}

void Program::UpdateCharts_optiontest(Charts chart)
{
    QVector<Point> points;

    qreal percent = ((mpi_.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valve_info = registry_->GetValveInfo();
    if (valve_info->normal_position != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - start_time_;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), mpi_[0]->GetPersent()});

    emit AddPoints(chart, points);
}

void Program::UpdateCharts_CyclicTred()
{
    QVector<Point> points;

    qreal percent = ((mpi_.GetDAC()->GetValue() - 4) / 16) * 100;
    percent = qMin(qMax(percent, 0.0), 100.0);

    ValveInfo *valve_info = registry_->GetValveInfo();
    if (valve_info->normal_position != 0) {
        percent = 100 - percent;
    }

    quint64 time = QDateTime::currentMSecsSinceEpoch() - start_time_;

    points.push_back({0, qreal(time), percent});
    points.push_back({1, qreal(time), mpi_[0]->GetPersent()});

    emit AddPoints(Charts::Cyclic, points);
}

void Program::MainTestResults(MainTest::TestResults results)
{
    ValveInfo *valve_info = registry_->GetValveInfo();

    qreal k = 5 * M_PI * valve_info->diameter * valve_info->diameter / 4;

    emit SetText(TextObjects::Label_pressure_diff,
                 QString::asprintf("%.3f bar", results.pressure_diff));
    emit SetText(TextObjects::Label_friction,
                 QString::asprintf("%.3f H", results.pressure_diff * k));
    emit SetText(TextObjects::Label_din_error_mean,
                 QString::asprintf("%.3f mA", results.din_error_mean));
    emit SetText(TextObjects::Label_din_error_max,
                 QString::asprintf("%.3f mA", results.din_error_max));
    emit SetText(TextObjects::Label_din_error_mean_percent,
                 QString::asprintf("%.2f %%", results.din_error_mean / 0.16));
    emit SetText(TextObjects::Label_din_error_max_percent,
                 QString::asprintf("%.2f %%", results.din_error_max / 0.16));
    emit SetText(TextObjects::Label_friction_percent,
                 QString::asprintf("%.2f %%", results.friction));
    emit SetText(TextObjects::Label_low_limit, QString::asprintf("%.2f bar", results.low_limit));
    emit SetText(TextObjects::Label_high_limit, QString::asprintf("%.2f bar", results.high_limit));

    emit SetText(TextObjects::LineEdit_dinamic_error,
                 QString::asprintf("%.2f", results.din_error_mean / 0.16));
    emit SetText(TextObjects::LineEdit_range_pressure,
                 QString::asprintf("%.2f - %.2f", results.low_limit, results.high_limit));
    emit SetText(TextObjects::LineEdit_range,
                 QString::asprintf("%.2f - %.2f", results.spring_low, results.spring_high));

    emit SetText(TextObjects::LineEdit_friction,
                 QString::asprintf("%.3f", results.pressure_diff * k));
    emit SetText(TextObjects::LineEdit_friction_percent,
                 QString::asprintf("%.2f", results.friction));
}

void Program::StepTestResults(QVector<StepTest::TestResult> results, quint32 T_value)
{
    emit SetStepResults(results, T_value);
}

void Program::GetPoints_maintest(QVector<QVector<QPointF> > &points)
{
    emit GetPoints(points, Charts::Main_task);
}

void Program::GetPoints_steptest(QVector<QVector<QPointF> > &points)
{
    emit GetPoints(points, Charts::Step);
}

void Program::EndTest()
{
    testing_ = false;
    emit EnableSetTask(true);
    emit SetButtonInitEnabled(true);
    SetDAC_int(0);
    emit StopTest();
    emit SetTask(mpi_.GetDAC()->GetValue());
}

void Program::SetDAC_real(qreal value)
{
    mpi_.SetDAC_Real(value);
}

void Program::SetDAC_int(quint16 value)
{
    mpi_.SetDAC_Raw(value);
}
void Program::SetBlockCTS(const SelectTests::BlockCTS &blockCTS)
{
    block_cts_ = blockCTS;
}
void Program::button_init()
{
    timer_sensors_->stop();
    timer_DI_->stop();

    emit SetButtonInitEnabled(false);
    emit SetSensorNumber(0);

    // emit SetGroupDOVisible(false);

    emit SetText(TextObjects::Label_status, "");
    emit SetText(TextObjects::Label_init, "");
    emit SetText(TextObjects::Label_sensors, "");
    emit SetText(TextObjects::Label_start_value, "");
    emit SetText(TextObjects::Label_end_value, "");

    // Инициализация: Статус устройства
    if (!mpi_.Connect()) {
        emit SetText(TextObjects::Label_status, "Ошибка подключения");
        emit SetTextColor(TextObjects::Label_status, Qt::red);
        emit SetButtonInitEnabled(true);
        return;
    }

    // Инициализация: Инициализация устройства
    emit SetText(TextObjects::Label_status, "Успешное подключение к порту " + mpi_.PortName());
    emit SetTextColor(TextObjects::Label_status, Qt::darkGreen);

    if (!mpi_.Initialize()) {
        emit SetText(TextObjects::Label_init, "Ошибка инициализации");
        emit SetTextColor(TextObjects::Label_status, Qt::red);
        emit SetButtonInitEnabled(true);
        return;
    }

    if ((mpi_.Version() & 0x40) != 0) {
       // emit SetGroupDOVisible(true);
       emit SetButtonsDOChecked(mpi_.GetDOStatus());
       timer_DI_->start();
    }

    emit SetText(TextObjects::Label_init, "Успешная инициализация");
    emit SetTextColor(TextObjects::Label_init, Qt::darkGreen);

    switch (mpi_.SensorCount()) { // update
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
                     "Обнаружено " + QString::number(mpi_.SensorCount()) + " датчика");
        emit SetTextColor(TextObjects::Label_sensors, Qt::darkGreen);
    }

    ValveInfo *valve_info = registry_->GetValveInfo();
    bool normal_closed = (valve_info->normal_position == 0);

    emit SetText(TextObjects::Label_start_value, "Измерение");
    emit SetTextColor(TextObjects::Label_start_value, Qt::darkYellow);

    SetDAC(0, 10000, true);

    QTimer timer(this);
    connect(&timer, &QTimer::timeout, this, [&] {
        if (!wait_for_button_ || stop_set_dac_) {
            dac_eventloop_->quit();
        }
    });

    timer.start(50);
    dac_eventloop_->exec();
    timer.stop();

    if (normal_closed)
        mpi_[0]->SetMin();
    else
        mpi_[0]->SetMax();

    emit SetText(TextObjects::Label_start_value, mpi_[0]->GetFormatedValue());
    emit SetTextColor(TextObjects::Label_start_value, Qt::darkGreen);

    emit SetText(TextObjects::Label_end_value, "Измерение");
    emit SetTextColor(TextObjects::Label_end_value, Qt::darkYellow);

    SetDAC(0xFFFF, 10000, true);

    timer.start(50);
    dac_eventloop_->exec();
    timer.stop();

    if (normal_closed)
        mpi_[0]->SetMax();
    else
        mpi_[0]->SetMin();

    emit SetText(TextObjects::Label_end_value, mpi_[0]->GetFormatedValue());
    emit SetTextColor(TextObjects::Label_end_value, Qt::darkGreen);

    qreal correct_coefficient = 1;
    if (valve_info->stroke_movement != 0) {
        correct_coefficient = qRadiansToDegrees(2 / valve_info->pulley);
        mpi_[0]->SetUnit("°");
    }
    mpi_[0]->CorrectCoefficients(correct_coefficient);

    if (normal_closed) {
        emit SetText(TextObjects::Label_range, mpi_[0]->GetFormatedValue());
        emit SetText(TextObjects::LineEdit_stroke, QString::asprintf("%.2f", mpi_[0]->GetValue()));
        SetDAC(0);
    } else {
        SetDAC(0, 10000, true);
        emit SetText(TextObjects::Label_range, mpi_[0]->GetFormatedValue());
        emit SetText(TextObjects::LineEdit_stroke, QString::asprintf("%.2f", mpi_[0]->GetValue()));
    }

    emit SetTask(mpi_.GetDAC()->GetValue());

    emit ClearPoints(Charts::Trend);
    init_time_ = QDateTime::currentMSecsSinceEpoch();

    emit SetSensorNumber(mpi_.SensorCount());
    emit SetButtonInitEnabled(true);
    timer_sensors_->start();
}

void Program::MainTestStart()
{
    MainTestSettings::TestParameters parameters;
    emit GetMainTestParameters(parameters);

    if (parameters.delay == 0) {
        emit StopTest();
        return;
    }

    parameters.dac_min = qMax(mpi_.GetDAC()->GetRawFromValue(parameters.signal_min),
                              mpi_.GetDac_Min());
    parameters.dac_max = qMin(mpi_.GetDAC()->GetRawFromValue(parameters.signal_max),
                              mpi_.GetDac_Max());

    emit SetButtonInitEnabled(false);

    MainTest *main_test = parameters.is_cyclic ? new CyclicTest : new MainTest;

    main_test->SetParameters(parameters);

    QThread *thread_test = new QThread(this);
    main_test->moveToThread(thread_test);

    if (parameters.is_cyclic) {
        connect(dynamic_cast<CyclicTest *>(main_test),
                &CyclicTest::UpdateCyclicTred,
                this,
                &Program::UpdateCharts_CyclicTred);
        connect(dynamic_cast<CyclicTest *>(main_test),
                &CyclicTest::SetStartTime,
                this,
                &Program::SetTimeStart);
    }

    connect(thread_test, &QThread::started, main_test, &MainTest::Process);
    connect(main_test, &MainTest::EndTest, thread_test, &QThread::quit);

    connect(this, &Program::StopTest, main_test, &MainTest::Stop);
    connect(thread_test, &QThread::finished, thread_test, &QThread::deleteLater);
    connect(thread_test, &QThread::finished, main_test, &MainTest::deleteLater);

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
        emit ClearPoints(Charts::Main_task);
        emit ClearPoints(Charts::Main_pressure);
        emit ClearPoints(Charts::Main_friction);
        emit SetRegressionEnable(false);
    });

    emit ClearPoints(Charts::Cyclic);

    testing_ = true;
    emit EnableSetTask(false);
    thread_test->start();
}

void Program::StrokeTestStart()
{
    emit SetButtonInitEnabled(false);
    emit ClearPoints(Charts::Stroke);

    StrokeTest *stroke_test = new StrokeTest;
    QThread *thread_test = new QThread(this);
    stroke_test->moveToThread(thread_test);

    connect(thread_test, &QThread::started, stroke_test, &StrokeTest::Process);
    connect(stroke_test, &StrokeTest::EndTest, thread_test, &QThread::quit);

    connect(this, &Program::StopTest, stroke_test, &StrokeTest::Stop);
    connect(thread_test, &QThread::finished, thread_test, &QThread::deleteLater);
    connect(thread_test, &QThread::finished, stroke_test, &StrokeTest::deleteLater);

    connect(this, &Program::ReleaseBlock, stroke_test, &MainTest::ReleaseBlock);

    connect(stroke_test, &StrokeTest::EndTest, this, &Program::EndTest);

    connect(stroke_test, &StrokeTest::UpdateGraph, this, &Program::UpdateCharts_stroketest);
    connect(stroke_test, &StrokeTest::SetDAC, this, &Program::SetDAC);
    connect(stroke_test, &StrokeTest::SetStartTime, this, &Program::SetTimeStart);
    connect(stroke_test, &StrokeTest::Results, this, &Program::StrokeTestResults);
    testing_ = true;
    emit EnableSetTask(false);
    thread_test->start();
}

void Program::OptionalTestStart(quint8 test_num)
{
    OptionTest::Task task;
    OptionTest *optional_test;
    ;

    switch (test_num) {
    case 0: {
        optional_test = new OptionTest;

        OtherTestSettings::TestParameters parameters;
        emit GetResponseTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optional_test;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valve_info = registry_->GetValveInfo();

        bool normal_open = (valve_info->normal_position != 0);

        task.value.push_back(mpi_.GetDAC()->GetRawFromValue(4.0));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            for (quint8 i = 0; i < 2; i++) {
                qreal current = 16.0 * (normal_open ? 100 - *it : *it) / 100 + 4.0;
                qreal dac_value = mpi_.GetDAC()->GetRawFromValue(current);
                task.value.push_back(dac_value);

                for (auto it_s = parameters.steps.begin(); it_s < parameters.steps.end(); ++it_s) {
                    current += (16 * *it_s / 100) * (i == 0 ? 1 : -1) * (normal_open ? -1 : 1);
                    dac_value = mpi_.GetDAC()->GetRawFromValue(current);
                    task.value.push_back(dac_value);
                }
            }
        }

        optional_test->SetTask(task);

        connect(optional_test, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optiontest(Charts::Response);
        });

        emit ClearPoints(Charts::Response);

        break;
    }
    case 1: {
        optional_test = new OptionTest;
        OtherTestSettings::TestParameters parameters;
        emit GetResolutionTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optional_test;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;

        ValveInfo *valve_info = registry_->GetValveInfo();

        bool normal_open = (valve_info->normal_position != 0);

        task.value.push_back(mpi_.GetDAC()->GetRawFromValue(4.0));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            qreal current = 16.0 * (normal_open ? 100 - *it : *it) / 100 + 4.0;
            qreal dac_value = mpi_.GetDAC()->GetRawFromValue(current);

            for (auto it_s = parameters.steps.begin(); it_s < parameters.steps.end(); ++it_s) {
                task.value.push_back(dac_value);
                current = (16 * (normal_open ? 100 - *it - *it_s : *it + *it_s) / 100 + 4.0);
                qreal dac_value_step = mpi_.GetDAC()->GetRawFromValue(current);
                task.value.push_back(dac_value_step);
            }
        }

        optional_test->SetTask(task);

        connect(optional_test, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optiontest(Charts::Resolution);
        });

        emit ClearPoints(Charts::Resolution);

        break;
    }

    case 2: {
        optional_test = new StepTest;
        StepTestSettings::TestParameters parameters;
        emit GetStepTestParameters(parameters);

        if (parameters.points.empty()) {
            delete optional_test;
            emit StopTest();
            return;
        }

        task.delay = parameters.delay;
        ValveInfo *valve_info = registry_->GetValveInfo();

        qreal start_value = 4.0;
        qreal end_value = 20.0;

        bool normal_open = (valve_info->normal_position != 0);

        task.value.push_back(mpi_.GetDAC()->GetRawFromValue(start_value));

        for (auto it = parameters.points.begin(); it != parameters.points.end(); ++it) {
            qreal current = 16.0 * (normal_open ? 100 - *it : *it) / 100 + 4.0;
            qreal dac_value = mpi_.GetDAC()->GetRawFromValue(current);
            task.value.push_back(dac_value);
        }

        task.value.push_back(mpi_.GetDAC()->GetRawFromValue(end_value));

        for (auto it = parameters.points.rbegin(); it != parameters.points.rend(); ++it) {
            qreal current = 16.0 * (normal_open ? 100 - *it : *it) / 100 + 4.0;
            qreal dac_value = mpi_.GetDAC()->GetRawFromValue(current);
            task.value.push_back(dac_value);
        }

        task.value.push_back(mpi_.GetDAC()->GetRawFromValue(start_value));

        optional_test->SetTask(task);
        dynamic_cast<StepTest *>(optional_test)->Set_T_value(parameters.test_value);

        connect(optional_test, &OptionTest::UpdateGraph, this, [&] {
            UpdateCharts_optiontest(Charts::Step);
        });
        connect(dynamic_cast<StepTest *>(optional_test),
                &StepTest::GetPoints,
                this,
                &Program::GetPoints_steptest,
                Qt::BlockingQueuedConnection);
        connect(dynamic_cast<StepTest *>(optional_test),
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

    QThread *thread_test = new QThread(this);
    optional_test->moveToThread(thread_test);

    emit SetButtonInitEnabled(false);

    connect(thread_test, &QThread::started, optional_test, &OptionTest::Process);
    connect(optional_test, &OptionTest::EndTest, thread_test, &QThread::quit);

    connect(this, &Program::StopTest, optional_test, &OptionTest::Stop);
    connect(thread_test, &QThread::finished, thread_test, &QThread::deleteLater);
    connect(thread_test, &QThread::finished, optional_test, &OptionTest::deleteLater);

    connect(this, &Program::ReleaseBlock, optional_test, &MainTest::ReleaseBlock);

    connect(optional_test, &OptionTest::EndTest, this, &Program::EndTest);

    connect(optional_test, &OptionTest::SetDAC, this, &Program::SetDAC);
    connect(optional_test, &OptionTest::SetStartTime, this, &Program::SetTimeStart);

    testing_ = true;
    emit EnableSetTask(false);
    thread_test->start();
}

void Program::TerminateTest()
{
    stop_set_dac_ = true;
    dac_eventloop_->quit();
    emit StopTest();
}


void Program::button_open() {}

void Program::button_report() {}

void Program::button_pixmap1() {}

void Program::button_pixmap2() {}

void Program::button_pixmap3() {}

void Program::button_set_position()
{
	stop_set_dac_ = true;
    dac_eventloop_->quit();
}

void Program::button_DO(quint8 DO_num, bool state)
{
    mpi_.SetDiscreteOutput(DO_num, state);
}

void Program::checkbox_autoinit(int state)
{
    wait_for_button_ = (state == 0);
}
