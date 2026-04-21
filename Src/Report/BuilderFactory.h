#pragma once

#include "Src/Report/Patterns/B_CVT.h"
#include "Src/Report/Patterns/B_SACVT.h"
#include "Src/Report/Patterns/C_CVT.h"
#include "Src/Report/Patterns/C_SACVT.h"
#include "Src/Report/Patterns/C_SOVT.h"
#include "Src/Gui/Setup/SelectTests.h"

namespace Report {
    class BuilderFactory
    {
    public:
        static std::unique_ptr<Builder> create(SelectTests::PatternType type)
        {
            switch (type)
            {
            case SelectTests::Pattern_B_CVT:
                return std::make_unique<Patterns::B_CVT>();

            case SelectTests::Pattern_B_SACVT:
                return std::make_unique<Patterns::B_SACVT>();

            case SelectTests::Pattern_C_CVT:
                return std::make_unique<Patterns::C_CVT>();

            case SelectTests::Pattern_C_SACVT:
                return std::make_unique<Patterns::C_SACVT>();

            case SelectTests::Pattern_C_SOVT:
                return std::make_unique<Patterns::C_SOVT>();
            default:
                return nullptr;
            }
        }
    };
}