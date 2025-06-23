#ifndef REPORTBUILDER_H
#define REPORTBUILDER_H

#pragma once
#include "ReportSaver.h"
#include "./Src/Telemetry/TelemetryStore.h"
#include <QWidget>

class ReportBuilder {
public:
    virtual ~ReportBuilder() = default;
    virtual void buildReport(
        ReportSaver::Report& report,
        const TelemetryStore& telemetryStore,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const QImage& image1 = QImage(),
        const QImage& image2 = QImage(),
        const QImage& image3 = QImage()
    ) = 0;

    inline QString safeToString(double val) {
        if (std::isnan(val) || std::isinf(val)) return "";
        if (val == 0.0) return "";
        return QString::number(val);
    }

    virtual QString templatePath() const = 0;
};

#endif // REPORTBUILDER_H
