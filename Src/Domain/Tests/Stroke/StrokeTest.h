#pragma once

#include "Src/Domain/Tests/Test.h"

class StrokeTest : public Test
{
    Q_OBJECT
public:
    explicit StrokeTest(QObject *parent = nullptr) : Test(parent) {}
    void run() override;

signals:
    void results();
};
