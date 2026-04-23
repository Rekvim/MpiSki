#pragma once

#include "Saver.h"
#include "Storage/Telemetry.h"
#include "Storage/Registry.h"

namespace Report {
    struct Context {
        const Telemetry& telemetry;
        const ObjectInfo& object;
        const ValveInfo& valve;
        const OtherParameters& params;
        const QImage& chartTask;
        const QImage& chartPressure;
        const QImage& chartFriction;
        const QImage& chartStep;
    };

    class Builder {
    public:
        virtual ~Builder() = default;
        virtual void buildReport(
            Saver::Report& report,
            const Telemetry& telemetryStore,
            const ObjectInfo& objectInfo,
            const ValveInfo& valveInfo,
            const OtherParameters& otherParams,
            const QImage& imageChartTask = QImage(),
            const QImage& imageChartPressure = QImage(),
            const QImage& imageChartFriction = QImage(),
            const QImage& imageChartStep = QImage()
        ) = 0;

        virtual QString templatePath() const = 0;

    protected:
        QString m_sheetStepReactionTest = "ТШР";
        QString m_sheetTechnicalInspection = "ТО";
        QString m_sheetCyclicTests = "ЦТ";
    };
}