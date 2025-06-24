#ifndef REPORTBUILDER_C_SOVT_H
#define REPORTBUILDER_C_SOVT_H

#pragma once
#include "ReportBuilder.h"
#include "Registry.h"

class ReportBuilder_C_SOVT : public ReportBuilder
{
public:
    ReportBuilder_C_SOVT();
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

    QString templatePath() const override { return ":/excel/Reports/SKI_C_SOVT.xlsx"; }
};

#endif // REPORTBUILDER_C_SOVT_H
