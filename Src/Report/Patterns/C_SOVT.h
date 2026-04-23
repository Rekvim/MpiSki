#pragma once

#include "Report/Builder.h"

namespace Report::Patterns {
    class C_SOVT : public Builder
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

        QString templatePath() const override { return ":/excel/Reports/SKI_C_SOVT.xlsx"; }
    };
}