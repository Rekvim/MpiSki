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
    void SetPatternType(SelectTests::PatternType pt);

    struct Task {
        QVector<quint16> values;
        QVector<quint16> sequence;
        quint16 cycles;
        quint32 delayMsecs;
        quint16 holdMsecs;
    };

    void SetTask(Task task);

    struct RangeRec {
        quint8 rangePercent = 0;
        qreal maxForwardValue = 0.0;
        int maxForwardCycle = -1;
        qreal maxReverseValue = 0.0;
        int maxReverseCycle = -1;
    };

    struct TestResults {
        QVector<RangeRec> ranges;
    };

    static QString seqToString(const QVector<quint16>& seq);

public slots:
    void Process() override;

signals:
    void errorOccured(const QString& text);
    void SetStartTime();
    void CycleCompleted(int completedCycles);
    void Results(TestResults results);

private:
    Task m_task;
    double processRegulatory();

    SelectTests::PatternType m_patternType;

    QVector<RangeRec> calculateRanges(const QVector<QVector<QPointF>>& pts,
                                      const QVector<quint16>& sequence) const;

    void fetchPoints(QVector<QVector<QPointF>>& pts);
    const bool m_endTestAfterProcess;
private:
    // Parameters m_params;
    QVector<quint16> m_doOnCounts;
    QVector<quint16> m_doOffCounts;
};

#endif // CYCLICTESTSREGULATORY_H
