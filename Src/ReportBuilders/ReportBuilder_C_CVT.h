#ifndef REPORTBUILDER_C_CVT_H
#define REPORTBUILDER_C_CVT_H

#pragma once
#include "ReportBuilder.h"
#include "./Program.h"


class ReportBuilder_C_CVT : public ReportBuilder
{
public:
    ReportBuilder_C_CVT();
    void buildReport(
        ReportSaver::Report& report,
        const TelemetryStore& telemetryStore,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const QImage& image1 = QImage(),
        const QImage& image2 = QImage(),
        const QImage& image3 = QImage()
        ) override;
    QString templatePath() const override { return ":/excel/Reports/SKI_C_CVT.xlsx"; }
};

#endif // REPORTBUILDER_C_CVT_H
