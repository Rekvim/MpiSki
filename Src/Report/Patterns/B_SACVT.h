#pragma once

#include "Src/Report/Builder.h"

namespace Report::Patterns {
    class B_SACVT : public Builder
    {
    public:
        void buildReport(
            Saver::Report& report,
            const Telemetry& telemetryStore,
            const ObjectInfo& objectInfo,
            const ValveInfo& valveInfo,
            const OtherParameters& otherParams,
            const QImage& imageChartTask = QImage(),
            const QImage& imageChartPressure = QImage(),
            const QImage& imageChartFriction = QImage(),
            const QImage& imageChartStep = QImage()
            ) override;
        QString templatePath() const override { return ":/excel/Reports/SKI_B_SACVT.xlsx"; }
    };
}