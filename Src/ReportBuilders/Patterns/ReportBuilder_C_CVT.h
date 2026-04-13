#pragma once
#include "Src/ReportBuilders/ReportBuilder.h"

class ReportBuilder_C_CVT : public ReportBuilder
{
public:
    ReportBuilder_C_CVT();
    void buildReport(
        ReportSaver::Report& report,
        const Telemetry& telemetryStore,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const QImage& imageChartTask = QImage(),
        const QImage& imageChartPressure = QImage(),
        const QImage& imageChartFriction = QImage(),
        const QImage& imageChartStep = QImage()
        ) override;
    QString templatePath() const override { return ":/excel/Reports/SKI_C_CVT.xlsx"; }
};
