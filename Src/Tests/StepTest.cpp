#include "StepTest.h"

StepTest::StepTest(QObject *parent)
    : OptionTest(parent, false)
{}

void StepTest::Process()
{
    OptionTest::Process();
    if (m_terminate) {
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
