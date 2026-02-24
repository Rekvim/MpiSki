#pragma once

struct ValveInfo;

#include "./Src/Ui/Setup/SelectTests.h"

class ITestSettings
{
public:
    virtual ~ITestSettings() = default;

    virtual void applyValveInfo(const ValveInfo& info) = 0;
    virtual void applyPattern(SelectTests::PatternType pattern) {}
};

