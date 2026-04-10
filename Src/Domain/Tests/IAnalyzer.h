#pragma once

#include "Src/Domain/Measurement/Sample.h"

class IAnalyzer
{
public:

    virtual ~IAnalyzer() = default;

    virtual void onSample(const Sample& s) = 0;

    virtual void reset() = 0;
};
