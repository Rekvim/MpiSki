#pragma once

#include "Src/Domain/Tests/Option/OptionTest.h"
#include <QPointF>

class StepTest : public OptionTest
{
    Q_OBJECT

public:
    explicit StepTest(QObject *parent = nullptr);
    void run() override;

    struct TestResult
    {
        quint16 from;
        quint16 to;
        quint32 T_value;
        qreal overshoot;
    };

    void setTValue(quint32 T_value);

private:
    QVector<TestResult> calculateResult(const QVector<QVector<QPointF>> &points) const;
    quint32 m_TValue;
signals:
    void getPoints(QVector<QVector<QPointF>> &points);
    void results(QVector<TestResult> result, quint32 T_value);
};
