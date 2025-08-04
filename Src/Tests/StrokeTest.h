#ifndef STROKETEST_H
#define STROKETEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>

#include "Test.h"
#include <QVector>
#include "SelectTests.h"

class StrokeTest : public Test
{
    Q_OBJECT

public:
    explicit StrokeTest(QObject *parent = nullptr);
    void Process() override;

    void SetPatternType(SelectTests::PatternType pattern);

signals:
    void SetMultipleDO(const QVector<bool>& states);
    void SetStartTime();
    void Results(quint64 forwardTime, quint64 backwardTime);

private:
    void processRegulatory();
    void processShutoff();

    SelectTests::PatternType m_patternType = SelectTests::Pattern_B_CVT;
};

#endif // STROKETEST_H
