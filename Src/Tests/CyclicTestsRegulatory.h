#ifndef CYCLICTESTSREGULATORY_H
#define CYCLICTESTSREGULATORY_H

#include "MainTest.h"
#include "SelectTests.h"

#include <QVector>
#include <QPointF>
#include <QString>

class CyclicTestsRegulatory : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestsRegulatory(QObject* parent = nullptr, bool endTestAfterProcess = true);

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

private:
    Task m_task;

    const bool m_endTestAfterProcess;
};

#endif // CYCLICTESTSREGULATORY_H
