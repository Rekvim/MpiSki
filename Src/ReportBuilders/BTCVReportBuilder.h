#ifndef BTCVREPORTBUILDER_H
#define BTCVREPORTBUILDER_H

#pragma once
#include "ReportBuilder.h"

class BTCVReportBuilder : public ReportBuilder
{
public:
    BTCVReportBuilder();
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
    QString templatePath() const override { return ":/excel/Reports/SKI_BTCV.xlsx"; }
};

#endif // BTCVREPORTBUILDER_H
