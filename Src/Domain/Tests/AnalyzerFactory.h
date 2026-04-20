#pragma once

#include "Src/Domain/Tests/IAnalyzer.h"

#include "Option/Step/StepAnalyzer.h"
#include "Stroke/StrokeAnalyzer.h"
#include "Main/MainAnalyzer.h"
#include "Cyclic/Regulatory/RegulatoryAnalyzer.h"
#include "Cyclic/Shutoff/ShutoffAnalyzer.h"
#include "Src/Domain/Program.h"

class AnalyzerFactory
{
public:
    static std::unique_ptr<IAnalyzer>
    create(Domain::Program::TestWorker type) {
        switch (type)
        {
        case Domain::Program::TestWorker::Stroke:
            return std::make_unique<Domain::Tests::Stroke::Analyzer>();
            break;

        case Domain::Program::TestWorker::Main:
            return std::make_unique<Domain::Tests::Main::Analyzer>();
            break;

        case Domain::Program::TestWorker::Step:
            return std::make_unique<Domain::Tests::Option::Step::Analyzer>();
            break;

        case Domain::Program::TestWorker::CyclicRegulatory:
            return std::make_unique<Domain::Tests::Cyclic::Regulatory::Analyzer>();
            break;

        case Domain::Program::TestWorker::CyclicShutOff:
            return std::make_unique<Domain::Tests::Cyclic::Shutoff::Analyzer>();
            break;
        default:
            return nullptr;
        }
    }
};