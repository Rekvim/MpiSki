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
        const QImage& imageChartTask = QImage(),
        const QImage& imageChartPressure = QImage(),
        const QImage& imageChartFriction = QImage(),
        const QImage& imageChartStep = QImage()
    ) = 0;

    virtual QString templatePath() const = 0;

protected:
    QString resultOk(bool ok) const {
        return ok ? "соответствует" : "не соответствует";
    }

    QString resultLimit(bool ok) const {
        return ok ? "не превышает" : "превышает";
    }

    void cell(ReportSaver::Report& report,
              const QString& sheet,
              quint16 row, quint16 col,
              const QVariant& value)
    {
        report.data.push_back({sheet, row, col, value.toString()});
    }

    void image(ReportSaver::Report& report,
               const QString& sheet,
               quint16 row, quint16 col,
               const QImage& img)
    {
        report.images.push_back({sheet, row, col, img});
    }
};

#endif // REPORTBUILDER_H
