#ifndef REPORTBUILDER_C_SACVT_H
#define REPORTBUILDER_C_SACVT_H

#pragma once
#include "ReportBuilder.h"

class ReportBuilder_C_SACVT : public ReportBuilder
{
public:
    ReportBuilder_C_SACVT();
    void buildReport(
        ReportSaver::Report& report,
        const TelemetryStore& telemetryStore,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const QImage& imageChartTask = QImage(),
        const QImage& imageChartPressure = QImage(),
        const QImage& imageChartFriction = QImage(),
        const QImage& imageChartStep = QImage()
        ) override;
    QString templatePath() const override { return ":/excel/Reports/SKI_C_SACVT.xlsx"; }
};

#endif // REPORTBUILDER_C_SACVT_H
