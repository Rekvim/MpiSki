#include "Algorithm.h"

namespace Domain::Tests::Option::Step {
void Algorithm::run()
{
    Option::Algorithm::run();
    if (m_terminate) {
        emit finished();
        return;
    }
    QVector<QVector<QPointF>> points;
    emit getPoints(points);
    emit results(calculateResult(points));
    emit finished();
}

void Algorithm::setTValue(quint32 T_value)
{
    m_TValue = T_value;
}

Result Algorithm::calculateResult(const QVector<QVector<QPointF>>& points) const
{
    Result result;
    result.testValue = m_TValue;
    const QVector<QPointF> &line = points.at(0);
    const QVector<QPointF> &task = points.at(1);

    qreal from = 0;
    qreal prevTask = task.first().y();
    qreal timeStart;
    quint32 t86Time = 0;
    qreal threshold;
    qreal overshoot = 0;
    bool t86Have = false;
    bool first = true;
    bool up;
    for (int i = 0; i < line.size(); ++i) {
        qreal currTask = task.at(i).y();
        if (!qFuzzyCompare(currTask, prevTask)) {
            if (first) {
                first = false;
            } else {
                result.steps.push_back({static_cast<quint16>(qRound(from)),
                                  static_cast<quint16>(qRound(prevTask)),
                                  t86Have ? t86Time : 0,
                                  overshoot});
            }
            from = prevTask;
            timeStart = task.at(i).x();
            threshold = (currTask - prevTask) * m_TValue / 100 + prevTask;
            up = (currTask > prevTask);
            overshoot = -1000;
            t86Have = false;
            prevTask = currTask;
        } else {
            if (first) {
                continue;
            }
            overshoot = qMax(overshoot, (line.at(i).y() - currTask) / (currTask - from) * 100);
            if (!t86Have) {
                if ((line.at(i).y() - threshold) * (up ? 1 : -1) > 0) {
                    t86Time = qRound(line.at(i).x() - timeStart);
                    t86Have = true;
                }
            }
        }
    }
    result.steps.push_back({static_cast<quint16>(qRound(from)),
                      static_cast<quint16>(qRound(prevTask)),
                      t86Have ? t86Time : 0,
                      overshoot});
    return result;
}
}
