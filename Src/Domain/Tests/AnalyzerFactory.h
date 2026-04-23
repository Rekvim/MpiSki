#pragma once

#include "Domain/Tests/IAnalyzer.h"

#include "Option/Step/Analyzer.h"
#include "Stroke/Analyzer.h"
#include "Main/Analyzer.h"
#include "Cyclic/Regulatory/Analyzer.h"
#include "Cyclic/Shutoff/Analyzer.h"
#include "Domain/Program.h"

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