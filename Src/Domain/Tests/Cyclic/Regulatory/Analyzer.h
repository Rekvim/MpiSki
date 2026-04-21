#pragma once

#include "Result.h"
#include "Src/Domain/Measurement/Sample.h"
#include "Src/Domain/Tests/IAnalyzer.h"

#include "../Params.h"
namespace Domain::Tests::Cyclic::Regulatory {

class Analyzer : public IAnalyzer
{
public:
    void configure(const Params& params);
    void start() override;
    void onSample(const Domain::Measurement::Sample& s) override;
    void finish() override;

    const Result& result() const;
private:
    QVector<int> m_ranges;
    Result m_result;
    int m_step = -1;
    int m_cycle = 0;
    bool m_isForward = true;
    double m_prevTask = NAN;

    int findStep(double task) const;
    void updateRange(double pos);
};
}