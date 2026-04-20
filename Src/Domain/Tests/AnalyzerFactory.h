#pragma once

#include "Src/Domain/Tests/IAnalyzer.h"

#include "Option/Step/StepTestAnalyzer.h"
#include "Stroke/StrokeTestAnalyzer.h"
#include "Main/Analyzer.h"
#include "Cyclic/Regulatory/Analyzer.h"
#include "Cyclic/Shutoff/Analyzer.h"
#include "Src/Domain/Program.h"

class AnalyzerFactory
{
public:

    static std::unique_ptr<IAnalyzer>
    create(Program::TestWorker type) {
        switch (type)
        {
        case Program::TestWorker::Step:
            return std::make_unique<StepTestAnalyzer>();
            break;

        case Program::TestWorker::Stroke:
            return std::make_unique<StrokeTestAnalyzer>();
            break;

        case Program::TestWorker::Main:
            return std::make_unique<Domain::Tests::Main::Analyzer>();
            break;

        case Program::TestWorker::CyclicRegulatory:
            return std::make_unique<Domain::Tests::Cyclic::Regulatory::Analyzer>();
            break;

        case Program::TestWorker::CyclicShutOff:
            return std::make_unique<Domain::Tests::Cyclic::Shutoff::Analyzer>();
            return nullptr;
            break;
        default:
            return nullptr;
        }
    }
};