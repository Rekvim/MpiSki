#pragma once

#include <memory>
#include <vector>

#include "IAnalyzer.h"
// #include "TestType.h"
#include "Step/Analyzer.h"
#include "Stroke/Analyzer.h"
#include "Main/Analyzer.h"
#include "Cyclic/Analyzer.h"

class AnalyzerFactory
{
public:



    // std::vector<std::unique_ptr<IAnalyzer>>
    // AnalyzerFactory::create(TestType type)
    // {
    //     std::vector<std::unique_ptr<IAnalyzer>> analyzers;

    //     switch (type)
    //     {
    //     case TestType::Step:
    //         analyzers.push_back(std::make_unique<StepAnalyzer>());
    //         break;

    //     case TestType::Stroke:
    //         analyzers.push_back(std::make_unique<StrokeAnalyzer>());
    //         break;

    //     case TestType::Main:
    //         analyzers.push_back(std::make_unique<MainAnalyzer>());
    //         break;

    //     case TestType::CyclicRegulatory:
    //         analyzers.push_back(std::make_unique<CyclicRegulatoryAnalyzer>());
    //         break;
    //     }

    //     return analyzers;
    // }
};