#ifndef CYCLICTESTSSHUTOFF_H
#define CYCLICTESTSSHUTOFF_H

#include "MainTest.h"
#include <QVector>
#include <QPointF>

class CyclicTestsShutoff : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestsShutoff(QObject* parent = nullptr);

    struct Task {
        QVector<quint16> values;
        quint64 delayMsecs;
        quint64 holdMsecs;
        quint16 cycles;
        QVector<bool> doMask;
    };

    struct TestResults {
        QVector<quint16> doOnCounts;
        QVector<quint16> doOffCounts;

        quint16 switch3to0Count = 0;
        quint16 switch0to3Count = 0;
    };

    void SetTask(const Task& task);

public slots:
    void Process() override;

signals:
    void CycleCompleted(int completedCycles);
    void Results(TestResults results);
    void SetMultipleDO(const QVector<bool>& states);
    void SetStartTime();

private:
    Task m_task;

    void calcSwitchCounts(const QVector<QVector<QPointF>>& pts,
                          int& s3to0, int& s0to3) const;

    void fetchPoints(QVector<QVector<QPointF>>& pts);
    QVector<quint16> m_doOnCounts, m_doOffCounts;
};

#endif // CYCLICTESTSSHUTOFF_H
