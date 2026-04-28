#pragma once

#include "Report/Builder.h"

namespace Report::Patterns {
    class B_SACVT : public Builder
    {
    public:
        void build(
            Saver::Report& report,
            const Telemetry& telemetryStore,
            const ObjectInfo& objectInfo,
            const ValveInfo& valveInfo,
            const OtherParameters& otherParams,
            const ChartImageStorage& chartImages
            ) override;
        QString templatePath() const override { return ":/excel/Reports/SKI_B_SACVT.xlsx"; }
    };
}