#ifndef REPORTBUILDER_B_CVT_H
#define REPORTBUILDER_B_CVT_H

#pragma once
#include "ReportBuilder.h"

class ReportBuilder_B_CVT : public ReportBuilder
{
public:
    ReportBuilder_B_CVT();
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
    QString templatePath() const override { return ":/excel/Reports/SKI_B_CVT.xlsx"; }
};

#endif // REPORTBUILDER_B_CVT_H
