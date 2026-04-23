#pragma once

#include "Domain/Tests/Test.h"
#include "Result.h"

namespace Domain::Tests::Cyclic::Shutoff {
    class Algorithm : public Test
    {
        Q_OBJECT
    public:
        explicit Algorithm(QObject* parent = nullptr) : Test(parent) {}

        struct Task {
            int delayMsecs = 0;
            int holdMsecs  = 0;
            int cycles = 0;
            int stepsPerCycle = 0;
            QVector<bool> doMask;
            QVector<quint16> values;
        };

        void setTask(const Task& task);

    public slots:
        void run() override;

    signals:
        void cycleCompleted(int completedCycles);
        void results(Result results);
        void SetMultipleDO(const QVector<bool>& states);

        void GetDI(quint8& di);
        void GetDO(quint8& doMask);

    private:
        Task m_task;
        QVector<quint16> m_doOnCounts, m_doOffCounts;
    };
}