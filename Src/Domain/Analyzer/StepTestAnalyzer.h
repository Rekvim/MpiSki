#pragma once

#include <QtGlobal>
#include <QVector>

#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/TestResults/StepTestResult.h"

class StepTestAnalyzer
{
private:
    QVector<Sample> m_samples;

public:
    void addSample(const Sample& s);

    const QVector<StepTestResult> result() const;
};
