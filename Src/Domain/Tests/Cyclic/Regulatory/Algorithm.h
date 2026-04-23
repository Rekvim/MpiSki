#pragma once

#include "Domain/Tests/Test.h"

namespace Domain::Tests::Cyclic::Regulatory {
    class Algorithm : public Test
    {
        Q_OBJECT
    public:
        explicit Algorithm(QObject* parent = nullptr) : Test(parent) {}

        struct Task {
            QVector<quint16> values;
            QVector<qreal> sequence;
            quint16 cycles;
            quint32 delayMsecs;
            quint16 holdMsecs;
        };

        void setTask(Task task);

    public slots:
        void run() override;

    signals:
        void errorOccured(const QString& text);
        void cycleCompleted(int completedCycles);

        void stepMeasured(int cycle, int step, bool forward);
        void results();

    private:
        Task m_task;
    };
}