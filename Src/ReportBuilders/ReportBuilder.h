#ifndef REPORTBUILDER_H
#define REPORTBUILDER_H

#pragma once
#include "ReportSaver.h"
#include <QWidget>

class ReportBuilder {
public:
    virtual ~ReportBuilder() = default;
    virtual void buildReport(
        ReportSaver::Report& report,
        const TestTelemetryData& telemetry,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const QImage& image1 = QImage(),
        const QImage& image2 = QImage(),
        const QImage& image3 = QImage()
        ) = 0;

    virtual QString templatePath() const = 0;
};

#endif // REPORTBUILDER_H
