#ifndef CYCLICTESTS_H
#define CYCLICTESTS_H

#include "MainTest.h"
#include "CyclicTestSettings.h"

#include <QVector>
#include <QPointF>
#include <QString>

class CyclicTests : public MainTest {
    Q_OBJECT
public:
    using Parameters = CyclicTestSettings::TestParameters;

    explicit CyclicTests(QObject* parent = nullptr);
    void SetParameters(const Parameters& params);

    struct RangeRec {
        quint16 rangePercent = 0;
        qreal maxForwardValue = 0.0;
        int maxForwardCycle = -1;
        qreal maxReverseValue = 0.0;
        int maxReverseCycle = -1;
    };

    struct TestResults {
        QString sequence;
        quint16 cycles = 0;
        double  totalTimeSec = 0.0;

        QVector<RangeRec> ranges;
        QVector<int> doOnCounts;
        QVector<int> doOffCounts;

        int switch3to0Count = 0;
        int switch0to3Count = 0;
    };

public slots:
    void Process() override;

signals:
    void SetMultipleDO(const QVector<bool>& states);
    void SetStartTime();
    void CycleCompleted(int completedCycles);
    void GetPoints(QVector<QVector<QPointF>>& points);
    void Results(TestResults results);

private:
    double processRegulatory();
    double processShutoff();

    QVector<RangeRec> calculateRanges(const QVector<QVector<QPointF>>& pts,
                                      const QVector<quint16>& sequence) const;
    void calcSwitchCounts(const QVector<QVector<QPointF>>& pts,
                          int& s3to0, int& s0to3) const;

    static QString seqToString(const QVector<quint16>& seq);
    void fetchPoints(QVector<QVector<QPointF>>& pts);

private:
    Parameters   m_params;
    QVector<int> m_doOnCounts;
    QVector<int> m_doOffCounts;
};

#endif // CYCLICTESTS_H
