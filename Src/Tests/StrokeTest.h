#pragma once

#include "Test.h"

class StrokeTest : public Test
{
    Q_OBJECT
public:
    explicit StrokeTest(QObject *parent = nullptr) : Test(parent) {}
    void Process() override;

signals:
    void Results();
};
