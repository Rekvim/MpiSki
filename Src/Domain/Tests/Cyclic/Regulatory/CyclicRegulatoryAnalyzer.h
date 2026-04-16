#pragma once

#include "CyclicRegulatoryTestResult.h"
#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/Tests/IAnalyzer.h"

#include "../CyclicTestParams.h"

class CyclicRegulatoryAnalyzer : public IAnalyzer
{
public:

    void configure(const CyclicTestParams& params);

    void start() override;
    void onSample(const Sample& s) override;
    void finish() override;

    const CyclicRegulatoryResult& result() const;
private:
    QVector<int> m_ranges;
    CyclicRegulatoryResult m_result;
    int m_step = -1;
    int m_cycle = 0;
    bool m_isForward = true;
    double m_prevTask = NAN;

    int findStep(double task) const;
    void updateRange(double pos);
};