#include "BTSVReportBuilder.h"

BTSVReportBuilder::BTSVReportBuilder() {}

void BTSVReportBuilder::buildReport(
    ReportSaver::Report& report,
    const TestTelemetryData& telemetry,
    const ObjectInfo& objectInfo,
    const ValveInfo& valveInfo,
    const OtherParameters& otherParams,
    const QImage& image1,
    const QImage& image2,
    const QImage& image3
    )
{
    // Страница: Отчет ЦТ; Блок: Данные по объекту
    report.data.push_back({"Отчет ЦТ", 5, 4, objectInfo.object});
    report.data.push_back({"Отчет ЦТ", 6, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет ЦТ", 7, 4, objectInfo.department});

    // Краткая спецификация на клапан
    report.data.push_back({"Отчет ЦТ", 5, 13, valveInfo.positionNumber});
    report.data.push_back({"Отчет ЦТ", 6, 13, valveInfo.serialNumber});
    report.data.push_back({"Отчет ЦТ", 7, 13, valveInfo.valveModel});
    report.data.push_back({"Отчет ЦТ", 8, 13, valveInfo.manufacturer});
    report.data.push_back({"Отчет ЦТ", 9, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Отчет ЦТ", 10, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет ЦТ", 11, 13, safeToString(telemetry.supplyPressure)});
    report.data.push_back({"Отчет ЦТ", 12, 13, safeToString(valveInfo.safePosition)});
    report.data.push_back({"Отчет ЦТ", 13, 13, safeToString(valveInfo.strokeMovement)});
    report.data.push_back({"Отчет ЦТ", 14, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет ЦТ", 15, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат циклического теста
    report.data.push_back({"Отчет ЦТ", 26, 8, safeToString(telemetry.strokeTest_timeForward)});
    report.data.push_back({"Отчет ЦТ", 28, 8, safeToString(telemetry.strokeTest_timeBackward)});
    report.data.push_back({"Отчет ЦТ", 30, 8, safeToString(telemetry.cyclicTest_cycles)});
    report.data.push_back({"Отчет ЦТ", 32, 8, safeToString(telemetry.cyclicTest_rangePercent)});
    report.data.push_back({"Отчет ЦТ", 34, 8, safeToString(telemetry.cyclicTest_totalTime)});

    // Страница:Отчет ЦТ; Блок: Выполненные задания позиционера
    // report.data.push_back({"Отчет ЦТ", 26, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 28, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 30, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 32, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 34, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 34, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 34, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 34, 8, safeToString(telemetry.???)});

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 62, 12, otherParams.date});

    // Страница:Отчет ЦТ; Блок: Концевеки и соленоиды
    // report.data.push_back({"Отчет ЦТ", 63, 5, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 64, 5, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 65, 5, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 66, 5, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 67, 5, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 68, 5, safeToString(telemetry.???)});

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 70, 4, objectInfo.FIO});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
