#pragma once

class ITestRunner
{
public:
    virtual ~ITestRunner() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
};