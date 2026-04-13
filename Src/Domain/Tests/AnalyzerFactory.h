#pragma once

#include "Src/Domain/Tests/IAnalyzer.h"

#include "Option/Step/StepTestAnalyzer.h"
#include "Stroke/StrokeTestAnalyzer.h"
#include "Main/MainTestAnalyzer.h"
#include "Cyclic/Regulatory/CyclicRegulatoryAnalyzer.h"
#include "Cyclic/Shutoff/CyclicShutoffAnalyzer.h"
#include "Src/Domain/Program.h"

class AnalyzerFactory
{
public:

    static std::unique_ptr<IAnalyzer>
    create(Program::Test type) {
        switch (type)
        {
        case Program::Test::Step:
            return std::make_unique<StepTestAnalyzer>();
            break;

        case Program::Test::Stroke:
            return std::make_unique<StrokeTestAnalyzer>();
            break;

        case Program::Test::Main:
            return std::make_unique<MainTestAnalyzer>();
            break;

        case Program::Test::CyclicRegulatory:
            return std::make_unique<CyclicRegulatoryAnalyzer>();
            break;

        case Program::Test::CyclicShutOff:
            return std::make_unique<CyclicShutoffAnalyzer>();
            return nullptr;
            break;
        default:
            return nullptr;
        }
    }
};