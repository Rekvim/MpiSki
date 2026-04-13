#pragma once

#include "Src/Domain/Tests/Test.h"

class CyclicTestsRegulatory : public Test
{
    Q_OBJECT
public:
    explicit CyclicTestsRegulatory(QObject* parent = nullptr);

    struct Task {
        QVector<quint16> values;
        QVector<qreal> sequence;
        quint16 cycles;
        quint32 delayMsecs;
        quint16 holdMsecs;
    };

    void SetTask(Task task);

public slots:
    void Process() override;

signals:
    void errorOccured(const QString& text);
    void SetStartTime();
    void CycleCompleted(int completedCycles);

    void StepMeasured(int cycle, int step, bool forward);
    void Results();

private:
    Task m_task;
};
