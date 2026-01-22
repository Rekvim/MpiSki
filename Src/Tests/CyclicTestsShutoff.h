#ifndef CYCLICTESTSSHUTOFF_H
#define CYCLICTESTSSHUTOFF_H

#include "MainTest.h"
#include <QVector>

class CyclicTestsShutoff : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestsShutoff(QObject* parent = nullptr);

    struct Task {
        int delayMsecs = 0;
        int holdMsecs  = 0;
        int cycles     = 0;
        int stepsPerCycle = 0;
        QVector<bool> doMask;
        QVector<quint16> values;
    };

    struct TestResults {
        QVector<quint16> doOnCounts;
        QVector<quint16> doOffCounts;
        int numCycles;
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

    // НОВОЕ: запросить у Program текущие маски (BlockingQueuedConnection)
    void GetDI(quint8& di);
    void GetDO(quint8& doMask);

private:
    Task m_task;

    QVector<quint16> m_doOnCounts, m_doOffCounts;
};

#endif // CYCLICTESTSSHUTOFF_H
