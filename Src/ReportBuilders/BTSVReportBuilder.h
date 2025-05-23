#ifndef BTSVREPORTBUILDER_H
#define BTSVREPORTBUILDER_H

#pragma once
#include "ReportBuilder.h"

class BTSVReportBuilder : public ReportBuilder
{
public:
    BTSVReportBuilder();
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
    QString templatePath() const override { return ":/excel/Reports/SKI_BTSV.xlsx"; }
};

#endif // BTSVREPORTBUILDER_H
