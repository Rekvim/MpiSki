#pragma once

#include <QtGlobal>
#include "Src/Domain/Measurement/TestDataBuffer.h"

struct StrokeTestResults
{
    quint64 forwardTimeMs = 0;
    quint64 backwardTimeMs = 0;
};

class StrokeTestAnalyzer
{
public:

    struct Config
    {
        bool normalClosed = true;
    };

    void setConfig(const Config& cfg);

    void start();

    void onSample(const Sample& s);

    StrokeTestResults finish();

private:
    Config m_cfg;
    QVector<Sample> m_samples;
};
