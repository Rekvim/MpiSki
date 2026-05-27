#pragma once

#include "Report/Patterns/B_CVT.h"
#include "Report/Patterns/B_SACVT.h"
#include "Report/Patterns/C_CVT.h"
#include "Report/Patterns/C_SACVT.h"
#include "Report/Patterns/C_SOVT.h"
#include "Domain/DeviceProfile.h"

namespace Report {
    class BuilderFactory
    {
    public:
        static std::unique_ptr<Builder> create(const Domain::DeviceProfile& p)
        {
            if (!p.hasShutoff() && !p.isComplex()) return std::make_unique<Patterns::B_CVT>();
            if (!p.hasShutoff() && p.isComplex()) return std::make_unique<Patterns::C_CVT>();
            if (p.hasShutoff() && p.hasControl() && !p.isComplex()) return std::make_unique<Patterns::B_SACVT>();
            if (p.hasShutoff() && p.hasControl() && p.isComplex()) return std::make_unique<Patterns::C_SACVT>();
            if (p.hasShutoff() && !p.hasControl()) return std::make_unique<Patterns::C_SOVT>();
            return nullptr;
        }
    };
}