#include "ReportBuilder_B_CVT.h"

ReportBuilder_B_CVT::ReportBuilder_B_CVT() {}

void ReportBuilder_B_CVT::buildReport(
    ReportSaver::Report& report,
    const TelemetryStore& telemetryStore,
    const ObjectInfo& objectInfo,
    const ValveInfo& valveInfo,
    const OtherParameters& otherParams,
    const QImage& image1,
    const QImage& image2,
    const QImage& image3
    )
{
    Q_UNUSED(image1);
    Q_UNUSED(image2);
    Q_UNUSED(image3);

    Q_UNUSED(image1);
    Q_UNUSED(image2);
    Q_UNUSED(image3);

    // Страница: Отчет ЦТ; Блок: Данные по объекту
    report.data.push_back({"Отчет ЦТ", 4, 4, objectInfo.object});
    report.data.push_back({"Отчет ЦТ", 5, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет ЦТ", 6, 4, objectInfo.department});

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    report.data.push_back({"Отчет ЦТ", 4, 13, valveInfo.positionNumber});
    report.data.push_back({"Отчет ЦТ", 5, 13, valveInfo.serialNumber});
    report.data.push_back({"Отчет ЦТ", 6, 13, valveInfo.valveModel});
    report.data.push_back({"Отчет ЦТ", 7, 13, valveInfo.manufacturer});
    report.data.push_back({"Отчет ЦТ", 8, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Отчет ЦТ", 9, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет ЦТ", 10, 13, telemetryStore.supplyRecord.supplyPressure});
    report.data.push_back({"Отчет ЦТ", 11, 13, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 12, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 13, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({"Отчет ЦТ", 19, 8, telemetryStore.strokeTestRecord.timeForward});
    report.data.push_back({"Отчет ЦТ", 21, 8, telemetryStore.strokeTestRecord.timeBackward});
    report.data.push_back({"Отчет ЦТ", 23, 8, telemetryStore.cyclicTestRecord.cycles});
    report.data.push_back({"Отчет ЦТ", 25, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({"Отчет ЦТ", 27, 8, telemetryStore.cyclicTestRecord.totalTime});

    // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
    // report.data.push_back({"Отчет ЦТ", 33, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 35, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 37, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 39, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 41, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 43, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 45, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 47, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 49, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 51, 8, safeToString(telemetry.???)});

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 56, 4, objectInfo.FIO});

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 60, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
