#pragma once

#include "Data.h"
#include "Storage/Telemetry.h"
#include "Storage/Registry.h"
#include "Storage/ChartImageStorage.h"

namespace Report {
    struct Context {
        const Telemetry& telemetry;
        const ObjectInfo& object;
        const ValveInfo& valve;
        const OtherParameters& params;
        const ChartImageStorage& chartImages;
    };

    class Builder {
    public:
        virtual ~Builder() = default;
        virtual void build(
            ReportData& report,
            const Telemetry& telemetryStore,
            const ObjectInfo& objectInfo,
            const ValveInfo& valveInfo,
            const OtherParameters& otherParams,
            const ChartImageStorage& chartImages
        ) = 0;

        virtual QString templatePath() const = 0;

    protected:
        QString m_sheetStepReactionTest = "ТШР";
        QString m_sheetTechnicalInspection = "ТО";
        QString m_sheetCyclicTests = "ЦТ";
    };
}