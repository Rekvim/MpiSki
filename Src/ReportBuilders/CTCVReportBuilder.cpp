#include "CTCVReportBuilder.h"

CTCVReportBuilder::CTCVReportBuilder() {}

void CTCVReportBuilder::buildReport(
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

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
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

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 62, 12, otherParams.date});

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 70, 4, objectInfo.FIO});


    // Страница: Отчет; Блок: Данные по объекту
    report.data.push_back({"Отчет", 5, 4, objectInfo.object});
    report.data.push_back({"Отчет", 6, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет", 7, 4, objectInfo.department});

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    report.data.push_back({"Отчет", 5, 13, valveInfo.positionNumber});
    report.data.push_back({"Отчет", 6, 13, valveInfo.serialNumber});
    report.data.push_back({"Отчет", 7, 13, valveInfo.valveModel});
    report.data.push_back({"Отчет", 8, 13, valveInfo.manufacturer});
    report.data.push_back({"Отчет", 9, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Отчет", 10, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет", 11, 13, safeToString(telemetry.supplyPressure)});
    report.data.push_back({"Отчет", 12, 13, safeToString(valveInfo.safePosition)});
    report.data.push_back({"Отчет", 13, 13, safeToString(valveInfo.strokeMovement)});
    report.data.push_back({"Отчет", 14, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет", 15, 13, valveInfo.materialStuffingBoxSeal});

    // Страница: Отчет; Блок: Результат испытаний
    report.data.push_back({"Отчет", 26, 5, safeToString(telemetry.dinamicReal)});
    report.data.push_back({"Отчет", 26, 8, safeToString(telemetry.dinamicRecomend)});

    report.data.push_back({"Отчет", 28, 5, safeToString(telemetry.dinamicIpReal)});
    report.data.push_back({"Отчет", 28, 8, safeToString(telemetry.dinamicIpRecomend)});

    report.data.push_back({"Отчет", 30, 5, safeToString(telemetry.strokeReal)});
    report.data.push_back({"Отчет", 30, 8, safeToString(telemetry.strokeRecomend)});

    report.data.push_back({"Отчет", 32, 5, safeToString(telemetry.rangeReal)});
    report.data.push_back({"Отчет", 32, 8, safeToString(telemetry.rangeRecomend)});

    report.data.push_back({"Отчет", 34, 5, safeToString(telemetry.rangePressure)});

    report.data.push_back({"Отчет", 36, 5, safeToString(telemetry.frictionPercent)});

    report.data.push_back({"Отчет", 38, 5, safeToString(telemetry.friction)});

    report.data.push_back({"Отчет", 52, 5, safeToString(telemetry.strokeTest_timeForward)});
    report.data.push_back({"Отчет", 52, 8, safeToString(telemetry.strokeTest_timeBackward)});


    // Страница: Отчет; Блок: Дата
    report.data.push_back({"Отчет", 66, 12, otherParams.date});

    // Страница: Отчет; Блок: Исполнитель
    report.data.push_back({"Отчет", 74, 4, objectInfo.FIO});

    // Страница: Отчет; Блок: Диагностические графики клапана, поз.
    report.images.push_back({"Отчет", 86, 1, image1});
    report.images.push_back({"Отчет", 111, 1, image2});
    report.images.push_back({"Отчет", 136, 1, image3});

    // Страница: Отчет; Блок: Дата
    report.data.push_back({"Отчет", 159, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
