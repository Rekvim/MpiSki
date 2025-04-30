#ifndef CYCLICTESTPOSITIONER_H
#define CYCLICTESTPOSITIONER_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>
#include "MainTest.h"

class CyclicTestPositioner : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestPositioner(QObject *parent = nullptr);
    void Process() override;

private:
    QTimer *m_cyclicGraphTimer;
signals:
    void UpdateCyclicTred();
    void SetStartTime();
};

#endif // CYCLICTESTPOSITIONER_H
