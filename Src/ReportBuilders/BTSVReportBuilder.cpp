#include "BTSVReportBuilder.h"

BTSVReportBuilder::BTSVReportBuilder() {}

void BTSVReportBuilder::buildReport( // gg
    ReportSaver::Report& report,
    const TestTelemetryData& telemetry,
    const ObjectInfo& objectInfo,
    const ValveInfo& valveInfo,
    const OtherParameters& otherParams,
    const QImage& image1,
    const QImage& image2,
    const QImage& image3
    ) {
    report.data.push_back({5, 4, objectInfo.object});
    report.data.push_back({6, 4, objectInfo.manufactory});
    report.data.push_back({7, 4, objectInfo.department});

    report.data.push_back({5, 13, valveInfo.positionNumber});
    report.data.push_back({6, 13, valveInfo.serialNumber});
    report.data.push_back({7, 13, valveInfo.valveModel});
    report.data.push_back({8, 13, valveInfo.manufacturer});
    report.data.push_back({9, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({10, 13, valveInfo.positionerModel});
    report.data.push_back({11, 13, QString::number(telemetry.supplyPressure)});
    report.data.push_back({12, 13, QString::number(valveInfo.safePosition)});
    report.data.push_back({13, 13, QString::number(valveInfo.strokeMovement)});
    report.data.push_back({14, 13, valveInfo.positionerModel});
    report.data.push_back({15, 13, valveInfo.materialStuffingBoxSeal});

    report.data.push_back({26, 8, QString::number(telemetry.supplyPressure)});
    report.data.push_back({28, 8, QString::number(telemetry.supplyPressure)});
    report.data.push_back({30, 8, QString::number(telemetry.supplyPressure)});
    report.data.push_back({32, 8, QString::number(telemetry.supplyPressure)});
    report.data.push_back({34, 8, QString::number(telemetry.supplyPressure)});


    report.data.push_back({62, 12, otherParams.date});
    report.data.push_back({70, 4, objectInfo.FIO});

    report.data.push_back({26, 5, QString::number(telemetry.dinamicReal)});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
