#pragma once

#include <QPointF>
#include <QTimer>

#include "Src/Domain/Tests/Main/MainTest.h"

class CyclicTestPositioner : public MainTest
{
    Q_OBJECT
public:
    explicit CyclicTestPositioner(QObject *parent = nullptr);
    void run() override;

private:
    QTimer *m_cyclicGraphTimer;
signals:
    void updateCyclicTred();
};
