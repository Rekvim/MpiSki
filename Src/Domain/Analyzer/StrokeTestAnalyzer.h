#pragma once

#include <QtGlobal>
#include <QVector>

#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/TestResults/StrokeTestResult.h"

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

    StrokeTestResult finish();

private:
    Config m_cfg;
    QVector<Sample> m_samples;
};
