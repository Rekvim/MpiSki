#ifndef CTVREPORTBUILDER_H
#define CTVREPORTBUILDER_H

#pragma once
#include "ReportBuilder.h"
#include "Registry.h"

class CTVReportBuilder : public ReportBuilder
{
public:
    CTVReportBuilder();
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

    QString templatePath() const override { return ":/excel/Reports/SKI_CTV.xlsx"; }
};

#endif // CTVREPORTBUILDER_H
