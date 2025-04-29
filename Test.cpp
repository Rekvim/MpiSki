#include <QDateTime>
#include <QEventLoop>
#include <QThread>

#include "Test.h"

Test::Test(QObject *parent)
    : QObject{parent}
{
    graph_timer_ = new QTimer(this);
    connect(graph_timer_, &QTimer::timeout, this, [&] { emit UpdateGraph(); });
    terminate_ = false;
    event_loop_ = new QEventLoop(this);
}

void Test::Sleep(quint16 msecs)
{
    QEventLoop loop(this);
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

void Test::SetDACBlocked(quint16 value, quint32 sleep_ms, bool wait_for_stop, bool wait_for_start)
{
    emit SetDAC(value, sleep_ms, wait_for_stop, wait_for_start);
    event_loop_->exec();
}

void Test::Stop()
{
    terminate_ = true;
    event_loop_->quit();
    graph_timer_->stop();
    //emit EndTest();
}

void Test::ReleaseBlock()
{
    event_loop_->quit();
}

MainTest::MainTest(QObject *parent, bool end_test_after_process)
    : Test(parent)
    , end_test_after_process_(end_test_after_process)
{}

void MainTest::Process()
{
    emit ClearGraph();
    emit ShowDots(!parameters_.continuous);

    SetDACBlocked(parameters_.dac_min, 10000, true);

    graph_timer_->start(parameters_.delay);

    Sleep(parameters_.delay);

    quint16 point_numbers = parameters_.point_numbers * parameters_.delay / parameters_.response;

    quint64 time;
    time = QDateTime::currentMSecsSinceEpoch();

    for (qint16 i = 0; i <= point_numbers; ++i) {
        quint16 dac = i * (parameters_.dac_max - parameters_.dac_min) / point_numbers
                      + parameters_.dac_min;

        time += parameters_.response;
        quint64 current_time = QDateTime::currentMSecsSinceEpoch();
        SetDACBlocked(dac, time < current_time ? 0 : (time - current_time));

        if (terminate_) {
            emit EndTest();
            return;
        }
    }

    SetDACBlocked(parameters_.dac_max, 0, true);

    emit DublSeries();

    Sleep(parameters_.delay);

    time = QDateTime::currentMSecsSinceEpoch();

    for (qint16 i = point_numbers; i >= 0; --i) {
        quint16 dac = i * (parameters_.dac_max - parameters_.dac_min) / point_numbers
                      + parameters_.dac_min;

        time += parameters_.response;
        quint64 current_time = QDateTime::currentMSecsSinceEpoch();
        SetDACBlocked(dac, time < current_time ? 0 : (time - current_time));

        if (terminate_) {
            emit EndTest();
            return;
        }
    }

    SetDACBlocked(parameters_.dac_min, 0, true);

    graph_timer_->stop();

    if (terminate_) {
        emit EndTest();
        return;
    }

    QVector<QVector<QPointF>> points;
    emit GetPoints(points);

    Limits regression_limits = GetLimits(points[2], points[3]);
    Regression regression_forward = CalculateRegression(points[2], regression_limits);
    Regression regression_backward = CalculateRegression(points[3], regression_limits);

    QVector<QPointF> points_forward = GetRegressionPoints(regression_forward, regression_limits);
    QVector<QPointF> points_backward = GetRegressionPoints(regression_backward, regression_limits);

    points_forward.append({points_backward.rbegin(), points_backward.rend()});
    points_forward.push_back(points_forward.first());

    emit AddRegression(points_forward);

    QVector<QPointF> friction_points = GetFrictionPoints(points[2], points[3], regression_limits);

    emit AddFriction(friction_points);

    TestResults test_results;

    qreal y_mean = (regression_limits.maxY + regression_limits.minY) / 2.0;

    test_results.pressure_diff = qAbs((y_mean - regression_forward.b) / regression_forward.k
                                      - (y_mean - regression_backward.b) / regression_backward.k);

    auto [mean, max] = GetMeanMax(points[0], points[1]);

    test_results.din_error_mean = mean / 2;
    test_results.din_error_max = max;

    qreal range = qAbs((regression_limits.minY - regression_forward.b) / regression_forward.k
                       - (regression_limits.maxY - regression_forward.b) / regression_forward.k);

    test_results.friction = 50.0 * test_results.pressure_diff / range;

    auto [low_limit, high_limit] = GetRangeLimits(regression_forward,
                                                  regression_backward,
                                                  regression_limits);

    test_results.low_limit = low_limit;
    test_results.high_limit = high_limit;

    auto [spring_low, spring_high] = GetSpringLimits(regression_forward,
                                                     regression_backward,
                                                     regression_limits);

    test_results.spring_low = spring_low;
    test_results.spring_high = spring_high;

    emit Results(test_results);
    if (end_test_after_process_) {
        emit EndTest();
    }
}

void MainTest::SetParameters(MainTestSettings::TestParameters &parameters)
{
    parameters_ = parameters;
}

MainTest::Regression MainTest::CalculateRegression(QVector<QPointF> &points, Limits limits)
{
    const qreal range = 0.10;

    qreal minY = (limits.maxY - limits.minY) * range + limits.minY;
    qreal maxY = limits.maxY - (limits.maxY - limits.minY) * range;

    qreal xy = 0;
    qreal x = 0;
    qreal y = 0;
    qreal x2 = 0;
    quint16 n = 0;

    foreach (QPointF P, points) {
        if ((P.y() >= minY) && (P.y() <= maxY)) {
            n++;
            xy += P.x() * P.y();
            x += P.x();
            y += P.y();
            x2 += qPow(P.x(), 2);
        }
    }

    Regression result;

    result.k = (n * xy - x * y) / (n * x2 - x * x);
    result.b = (y - result.k * x) / n;

    return result;
}

MainTest::Limits MainTest::GetLimits(const QVector<QPointF> &points1,
                                     const QVector<QPointF> &points2)
{
    Limits result;

    auto [minX1, maxX1] = std::minmax_element(points1.begin(),
                                              points1.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.x() < b.x();
                                              });

    auto [minX2, maxX2] = std::minmax_element(points2.begin(),
                                              points2.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.x() < b.x();
                                              });

    result.minX = qMin(minX1->x(), minX2->x());
    result.maxX = qMax(maxX1->x(), maxX2->x());

    auto [minY1, maxY1] = std::minmax_element(points1.begin(),
                                              points1.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.y() < b.y();
                                              });

    auto [minY2, maxY2] = std::minmax_element(points2.begin(),
                                              points2.end(),
                                              [](QPointF a, QPointF b) -> bool {
                                                  return a.y() < b.y();
                                              });

    result.minY = qMin(minY1->y(), minY2->y());
    result.maxY = qMax(maxY1->y(), maxY2->y());

    return result;
}

QVector<QPointF> MainTest::GetRegressionPoints(Regression regression, Limits limits)
{
    QPointF point_minX(limits.minX, regression.k * limits.minX + regression.b);
    QPointF point_maxX(limits.maxX, regression.k * limits.maxX + regression.b);
    QPointF point_minY((limits.minY - regression.b) / regression.k, limits.minY);
    QPointF point_maxY((limits.maxY - regression.b) / regression.k, limits.maxY);

    auto PointInLimits = [limits](QPointF point) {
        return (point.x() >= limits.minX) && (point.x() <= limits.maxX)
               && (point.y() >= limits.minY) && (point.y() <= limits.maxY);
    };

    bool minX_InLimits = PointInLimits(point_minX);
    bool maxX_InLimits = PointInLimits(point_maxX);
    bool minY_InLimits = PointInLimits(point_minY);
    bool maxY_InLimits = PointInLimits(point_maxY);

    QVector<QPointF> result;

    if (minX_InLimits && maxX_InLimits) {
        result.push_back(point_minX);
        result.push_back(point_maxX);
        return result;
    }

    if (minX_InLimits && minY_InLimits) {
        result.push_back(point_minX);
        result.push_back(point_minY);
        result.push_back({limits.maxX, limits.minY});
        return result;
    }

    if (minX_InLimits && maxY_InLimits) {
        result.push_back(point_minX);
        result.push_back(point_maxY);
        result.push_back({limits.maxX, limits.maxY});
        return result;
    }

    if (maxX_InLimits && minY_InLimits) {
        result.push_back({limits.minX, limits.minY});
        result.push_back(point_minY);
        result.push_back(point_maxX);
        return result;
    }

    if (maxX_InLimits && maxY_InLimits) {
        result.push_back({limits.minX, limits.maxY});
        result.push_back(point_maxY);
        result.push_back(point_maxX);
        return result;
    }

    if (minY_InLimits && maxY_InLimits) {
        if (point_minY.x() < point_maxY.x()) {
            result.push_back({limits.minX, limits.minY});
            result.push_back(point_minY);
            result.push_back(point_maxY);
            result.push_back({limits.maxX, limits.maxY});
        } else {
            result.push_back({limits.minX, limits.maxY});
            result.push_back(point_maxY);
            result.push_back(point_minY);
            result.push_back({limits.maxX, limits.minY});
        }
    }

    return result;
}

QVector<QPointF> MainTest::GetFrictionPoints(QVector<QPointF> &points_forward,
                                             QVector<QPointF> &points_backward,
                                             Limits limits)
{
    const quint16 Sections = qMin(points_forward.size(), points_backward.size()) / 3;

    qreal step = (limits.maxY - limits.minY) / Sections;

    QVector<quint16> points_num_forward(Sections);
    QVector<quint16> points_num_backward(Sections);
    QVector<qreal> points_value_forward(Sections);
    QVector<qreal> points_value_backward(Sections);

    for (auto point : points_forward) {
        quint16 section_num = qFloor((point.y() - limits.minY) / step);
        section_num = qMin(section_num, quint16(Sections - 1));
        ++points_num_forward[section_num];
        points_value_forward[section_num] += point.x();
    }

    for (auto point : points_backward) {
        quint16 section_num = qFloor((point.y() - limits.minY) / step);
        section_num = qMin(section_num, quint16(Sections - 1));
        ++points_num_backward[section_num];
        points_value_backward[section_num] += point.x();
    }

    QVector<QPointF> result;

    for (quint16 i = Sections * 0.05; i < Sections * 0.95; ++i) {
        if (points_num_forward[i] == 0 || points_num_backward[i] == 0)
            continue;
        result.push_back({step * i + limits.minY,
                          qAbs(points_value_forward[i] / points_num_forward[i]
                               - points_value_backward[i] / points_num_backward[i])});
    }

    return result;
}

QPair<qreal, qreal> MainTest::GetMeanMax(QVector<QPointF> &points_forward,
                                         QVector<QPointF> &points_backward)
{
    Limits limits = GetLimits(points_forward, points_backward);

    const quint16 Sections = qMin(points_forward.size(), points_backward.size()) / 3;

    qreal step = (limits.maxY - limits.minY) / Sections;

    QVector<quint16> points_num_forward(Sections);
    QVector<quint16> points_num_backward(Sections);
    QVector<qreal> points_value_forward(Sections);
    QVector<qreal> points_value_backward(Sections);

    for (auto point : points_forward) {
        quint16 section_num = qFloor((point.y() - limits.minY) / step);
        section_num = qMin(section_num, quint16(Sections - 1));
        ++points_num_forward[section_num];
        points_value_forward[section_num] += point.x();
    }

    for (auto point : points_backward) {
        quint16 section_num = qFloor((point.y() - limits.minY) / step);
        section_num = qMin(section_num, quint16(Sections - 1));
        ++points_num_backward[section_num];
        points_value_backward[section_num] += point.x();
    }

    qreal sum = 0;
    quint16 num = 0;
    qreal max = 0;

    for (quint16 i = Sections * 0.05; i < Sections * 0.95; ++i) {
        if (points_num_forward[i] == 0 || points_num_backward[i] == 0)
            continue;

        qreal diff = qAbs(points_value_forward[i] / points_num_forward[i]
                          - points_value_backward[i] / points_num_backward[i]);

        sum += diff;
        ++num;
        max = qMax(max, diff);
    }

    return qMakePair(sum / num, max);
}

QPair<qreal, qreal> MainTest::GetRangeLimits(Regression regression1,
                                             Regression regression2,
                                             Limits limits)
{
    auto x_val = [](Regression regression, qreal y) { return (y - regression.b) / regression.k; };

    qreal x1 = x_val(regression1, limits.minY);
    qreal x2 = x_val(regression1, limits.maxY);
    qreal x3 = x_val(regression2, limits.minY);
    qreal x4 = x_val(regression2, limits.maxY);

    qreal min = qMin(qMin(x1, x2), qMin(x3, x4));
    qreal max = qMax(qMax(x1, x2), qMax(x3, x4));

    return qMakePair(min, max);
}

QPair<qreal, qreal> MainTest::GetSpringLimits(Regression regression1,
                                              Regression regression2,
                                              Limits limits)
{
    auto x_val = [](Regression regression, qreal y) { return (y - regression.b) / regression.k; };

    qreal x1 = x_val(regression1, limits.minY);
    qreal x2 = x_val(regression1, limits.maxY);
    qreal x3 = x_val(regression2, limits.minY);
    qreal x4 = x_val(regression2, limits.maxY);

    qreal min = (qMin(x1, x2) + qMin(x3, x4)) / 2;
    qreal max = (qMax(x1, x2) + qMax(x3, x4)) / 2;

    return qMakePair(min, max);
}

StrokeTest::StrokeTest(QObject *parent)
    : Test(parent)
{}

void StrokeTest::Process()
{
    SetDACBlocked(0, 10000, true);

    if (terminate_) {
        emit EndTest();
        return;
    }

    emit SetStartTime();

    graph_timer_->start(50);

    Sleep(5000);

    if (terminate_) {
        emit EndTest();
        return;
    }

    quint64 start_time = QDateTime::currentMSecsSinceEpoch();

    SetDACBlocked(0xFFFF, 0, true, true);

    if (terminate_) {
        emit EndTest();
        return;
    }

    quint64 forward_time = QDateTime::currentMSecsSinceEpoch() - start_time - 2500;

    start_time = QDateTime::currentMSecsSinceEpoch();

    SetDACBlocked(0, 0, true, true);

    if (terminate_) {
        emit EndTest();
        return;
    }

    quint64 backward_time = QDateTime::currentMSecsSinceEpoch() - start_time - 2500;

    emit Results(forward_time, backward_time);

    emit EndTest();
}

OptionTest::OptionTest(QObject *parent, bool end_test_after_process)
    : Test(parent)
    , end_test_after_process_(end_test_after_process)
{}

void OptionTest::Process()
{
    if (task_.value.empty()) {
        emit EndTest();
        return;
    }

    SetDACBlocked(task_.value.first(), 10000, true);

    if (terminate_) {
        emit EndTest();
        return;
    }

    emit SetStartTime();

    graph_timer_->start(50);

    for (const auto value : task_.value) {
        SetDACBlocked(value, task_.delay);

        if (terminate_) {
            emit EndTest();
            return;
        }
    }
    if (end_test_after_process_) {
        emit EndTest();
    }
}

void OptionTest::SetTask(Task task)
{
    task_ = task;
}

StepTest::StepTest(QObject *parent)
    : OptionTest(parent, false)
{}

void StepTest::Process()
{
    OptionTest::Process();
    if (terminate_) {
        emit EndTest();
        return;
    }
    QVector<QVector<QPointF>> points;
    emit GetPoints(points);
    emit Results(CalculateResult(points), T_value_);
    emit EndTest();
}

void StepTest::Set_T_value(quint32 T_value)
{
    T_value_ = T_value;
}

QVector<StepTest::TestResult> StepTest::CalculateResult(const QVector<QVector<QPointF>> &points) const
{
    QVector<TestResult> result;
    const QVector<QPointF> &line = points.at(0);
    const QVector<QPointF> &task = points.at(1);

    qreal from = 0;
    qreal prev_task = task.first().y();
    qreal time_start;
    quint64 t86_time = 0;
    qreal threshold;
    qreal overshoot = 0;
    bool have_t86 = false;
    bool first = true;
    bool up;
    for (int i = 0; i < line.size(); ++i) {
        qreal curr_task = task.at(i).y();
        if (!qFuzzyCompare(curr_task, prev_task)) {
            if (first) {
                first = false;
            } else {
                result.push_back({static_cast<quint16>(qRound(from)),
                                  static_cast<quint16>(qRound(prev_task)),
                                  have_t86 ? t86_time : 0,
                                  overshoot});
            }
            from = prev_task;
            time_start = task.at(i).x();
            threshold = (curr_task - prev_task) * T_value_ / 100 + prev_task;
            up = (curr_task > prev_task);
            overshoot = -1000;
            have_t86 = false;
            prev_task = curr_task;
        } else {
            if (first) {
                continue;
            }
            overshoot = qMax(overshoot, (line.at(i).y() - curr_task) / (curr_task - from) * 100);
            if (!have_t86) {
                if ((line.at(i).y() - threshold) * (up ? 1 : -1) > 0) {
                    t86_time = qRound(line.at(i).x() - time_start);
                    have_t86 = true;
                }
            }
        }
    }
    result.push_back({static_cast<quint16>(qRound(from)),
                      static_cast<quint16>(qRound(prev_task)),
                      have_t86 ? t86_time : 0,
                      overshoot});
    return result;
}

CyclicTest::CyclicTest(QObject *parent)
    : MainTest(parent, false)
{}

void CyclicTest::Process()
{
    cyclic_graph_timer_ = new QTimer(this);
    connect(cyclic_graph_timer_, &QTimer::timeout, this, [&] { emit UpdateCyclicTred(); });

    emit SetStartTime();
    cyclic_graph_timer_->start(500);
    MainTest::Process();

    for (int i = 0; i < parameters_.num_cycles; ++i) {
        if (terminate_) {
            emit EndTest();
            return;
        }
        SetDACBlocked(0xFFFF, 0, true, true);
        if (terminate_) {
            emit EndTest();
            return;
        }
        SetDACBlocked(0, 0, true, true);
    }

    if (terminate_) {
        emit EndTest();
        return;
    }

    MainTest::Process();
    emit EndTest();
}

// CyclicTestSolenoid::CyclicTestSolenoid(QObject *parent) // new
//     : MainTest(parent, false)
// {}

// void CyclicTestSolenoid::Process()
// {
//     // emit SetStartTime();                        // обнулим таймер начала

//     // CyclicTestSettings::TestParameters params;

//     // QElapsedTimer total_timer;
//     // total_timer.start();

//     // quint64 forward_time = 0;
//     // quint64 backward_time = 0;

//     // for (quint16 cycle = 0; cycle < params.num_cycles && !terminate_; ++cycle) {
//     //     QElapsedTimer cycle_timer;
//     //     cycle_timer.start();

//     //     // Проход по шагам — вперед
//     //     for (int i = 0; i < params.points.size(); ++i) {
//     //         const qreal value = params.points[i];
//     //         const quint16 dac = static_cast<quint16>(value * 10); // 100% → 1000 DAC

//     //         SetDACBlocked(dac, 10000); // пауза 10 сек

//     //         qreal elapsed_sec = total_timer.elapsed() / 1000.0;
//     //         emit AddPoints(Charts::Cyclic_solenoid, {{0, elapsed_sec, value}});
//     //     }

//     //     // Проход по шагам — обратно
//     //     for (int i = params.points.size() - 2; i >= 0; --i) { // -2 чтобы не дублировать центральную точку
//     //         const qreal value = params.points[i];
//     //         const quint16 dac = static_cast<quint16>(value * 10);Хоро

//     //         SetDACBlocked(dac, 10000);

//     //         qreal elapsed_sec = total_timer.elapsed() / 1000.0;
//     //         emit AddPoints(Charts::Cyclic_solenoid, {{0, elapsed_sec, value}});
//     //     }

//     //     forward_time += cycle_timer.elapsed(); // пока считаем всё как forward
//     // }

//     // // Итоги
//     // params.time_forward_last = forward_time;
//     // params.time_backward_last = backward_time;  // можешь потом сделать раздельный замер
//     // params.total_time = total_timer.elapsed();
//     // params.num_cycles_done = params.num_cycles;
//     // params.range_percent = params.points.last() - params.points.first();

//     // emit SetSolenoidResults(forward_time,
//     //                         backward_time,
//     //                         params.num_cycles_done,
//     //                         params.range_percent,
//     //                         params.total_time / 1000.0);

//     // emit EndTest();
// }
