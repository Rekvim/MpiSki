#ifndef CTSVREPORTBUILDER_H
#define CTSVREPORTBUILDER_H

#pragma once
#include "ReportBuilder.h"

class CTSVReportBuilder  : public ReportBuilder
{
public:
    CTSVReportBuilder();
    void buildReport(
        ReportSaver::Report& report,
        const TestTelemetryData& telemetry,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const QImage& image1 = QImage(),
        const QImage& image2 = QImage(),
        const QImage& image3 = QImage()
        ) override;
    QString templatePath() const override { return ":/excel/SKI_CTSV.xlsx"; }
};

#endif // CTSVREPORTBUILDER_H
