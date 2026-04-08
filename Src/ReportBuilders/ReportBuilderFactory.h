#pragma once

#include "Src/ReportBuilders/Patterns/ReportBuilder_B_CVT.h"
#include "Src/ReportBuilders/Patterns/ReportBuilder_B_SACVT.h"
#include "Src/ReportBuilders/Patterns/ReportBuilder_C_CVT.h"
#include "Src/ReportBuilders/Patterns/ReportBuilder_C_SACVT.h"
#include "Src/ReportBuilders/Patterns/ReportBuilder_C_SOVT.h"
#include "Src/Ui/Setup/SelectTests.h"

class ReportBuilderFactory
{
public:
    static std::unique_ptr<ReportBuilder> create(SelectTests::PatternType type)
    {
        switch (type)
        {
        case SelectTests::Pattern_B_CVT:
            return std::make_unique<ReportBuilder_B_CVT>();

        case SelectTests::Pattern_B_SACVT:
            return std::make_unique<ReportBuilder_B_SACVT>();

        case SelectTests::Pattern_C_CVT:
            return std::make_unique<ReportBuilder_C_CVT>();

        case SelectTests::Pattern_C_SACVT:
            return std::make_unique<ReportBuilder_C_SACVT>();

        case SelectTests::Pattern_C_SOVT:
            return std::make_unique<ReportBuilder_C_SOVT>();
        default:
            return nullptr;
        }
    }
};