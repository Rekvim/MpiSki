#ifndef STROKETEST_H
#define STROKETEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>
#include "Test.h"

class StrokeTest : public Test
{
    Q_OBJECT

public:
    explicit StrokeTest(QObject *parent = nullptr);
    void Process() override;
signals:
    void SetStartTime();
    void Results(quint64 forward_time, quint64 backward_time);
};

#endif // STROKETEST_H
