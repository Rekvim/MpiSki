#pragma once

#include "Report/Builder.h"

namespace Report::Patterns {
    class C_SACVT : public Builder
    {
    public:
        void build(
            ReportData& report,
            const Telemetry& telemetryStore,
            const ObjectInfo& objectInfo,
            const ValveInfo& valveInfo,
            const OtherParameters& otherParams,
            const ChartImageStorage& chartImages
            ) override;
        QString templatePath() const override { return ":/excel/Reports/SKI_C_SACVT.xlsx"; }
    };
}