#include "BTCVReportBuilder.h"

BTCVReportBuilder::BTCVReportBuilder() {}

void BTCVReportBuilder::buildReport(
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
    Q_UNUSED(image1);
    Q_UNUSED(image2);
    Q_UNUSED(image3);

    // Лист 1; Страница: Отчет ЦТ; Блок: Данные по объекту
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
    report.data.push_back({"Отчет ЦТ", 10, 13, valveInfo.solenoidValveModel});
    report.data.push_back({"Отчет ЦТ", 11, 13, valveInfo.limitSwitchModel + "/" + valveInfo.materialStuffingBoxSeal});
    report.data.push_back({"Отчет ЦТ", 12, 13, safeToString(telemetry.supplyPressure)});
    report.data.push_back({"Отчет ЦТ", 13, 13, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 14, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 15, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({"Отчет ЦТ", 21, 8, safeToString(telemetry.strokeTest_timeForward)}); // Результат теста полного хода
    report.data.push_back({"Отчет ЦТ", 23, 8, safeToString(telemetry.strokeTest_timeBackward)}); // Результат теста полного хода
    report.data.push_back({"Отчет ЦТ", 25, 8, safeToString(telemetry.cyclicTest_cycles)});
    // report.data.push_back({"Отчет ЦТ", 27, 8, safeToString(telemetry.cyclicTest_rangePercent)}); // Указывается выбранное значение ЦТ (по умолчанию либо введеный вручную)
    // report.data.push_back({"Отчет ЦТ", 29, 8, safeToString(telemetry.cyclicTest_totalTime)});

    // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
    // report.data.push_back({"Отчет ЦТ", 35, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 37, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 39, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 41, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 43, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 45, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 47, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 49, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 51, 8, safeToString(telemetry.???)});
    // report.data.push_back({"Отчет ЦТ", 53, 8, safeToString(telemetry.???)});

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 58, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 62, 12, otherParams.date});

    // Лист 2; Страница: Отчет ЦТ; Блок: Данные по объекту
    report.data.push_back({"Отчет ЦТ", 68, 4, objectInfo.object});
    report.data.push_back({"Отчет ЦТ", 69, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет ЦТ", 70, 4, objectInfo.department});

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    report.data.push_back({"Отчет ЦТ", 68, 13, valveInfo.positionNumber});
    report.data.push_back({"Отчет ЦТ", 69, 13, valveInfo.serialNumber});
    report.data.push_back({"Отчет ЦТ", 70, 13, valveInfo.valveModel});
    report.data.push_back({"Отчет ЦТ", 71, 13, valveInfo.manufacturer});
    report.data.push_back({"Отчет ЦТ", 72, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Отчет ЦТ", 73, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет ЦТ", 74, 13, valveInfo.solenoidValveModel});
    report.data.push_back({"Отчет ЦТ", 75, 13, valveInfo.limitSwitchModel + "/" + valveInfo.materialStuffingBoxSeal});
    report.data.push_back({"Отчет ЦТ", 76, 13, safeToString(telemetry.supplyPressure)});
    report.data.push_back({"Отчет ЦТ", 77, 13, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 78, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 79, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 80, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    report.data.push_back({"Отчет ЦТ", 85, 8, safeToString(telemetry.strokeTest_timeForward)}); // Результат теста полного хода
    report.data.push_back({"Отчет ЦТ", 87, 8, safeToString(telemetry.strokeTest_timeBackward)}); // Результат теста полного хода
    report.data.push_back({"Отчет ЦТ", 89, 8, safeToString(telemetry.cyclicTest_cycles)});
    // report.data.push_back({"Отчет ЦТ", 91, 8, safeToString(telemetry.cyclicTest_rangePercent)}); // Указывается выбранное значение ЦТ (по умолчанию либо введеный вручную)
    // report.data.push_back({"Отчет ЦТ", 93, 8, safeToString(telemetry.cyclicTest_totalTime)});

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    // report.data.push_back({"Отчет ЦТ", 101, 8, safeToString(???)}); // Задание диапазона (0% хода)
    // report.data.push_back({"Отчет ЦТ", 103, 8, safeToString(???)}); // Задание диапазона (100% хода)

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    // report.data.push_back({"Отчет ЦТ", 109, 8, safeToString(???)}); // Положение "О-3"
    // report.data.push_back({"Отчет ЦТ", 111, 8, safeToString(???)}); // Положение "З-О"

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 118, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 122, 12, otherParams.date});


    // Страница: Отчет; Блок: Данные по объекту
    report.data.push_back({"Отчет", 4, 4, objectInfo.object});
    report.data.push_back({"Отчет", 5, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет", 6, 4, objectInfo.department});

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    report.data.push_back({"Отчет ЦТ", 4, 14, valveInfo.positionNumber});
    report.data.push_back({"Отчет ЦТ", 5, 14, valveInfo.serialNumber});
    report.data.push_back({"Отчет ЦТ", 6, 14, valveInfo.valveModel});
    report.data.push_back({"Отчет ЦТ", 7, 14, valveInfo.manufacturer});
    report.data.push_back({"Отчет ЦТ", 8, 14, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Отчет ЦТ", 9, 14, valveInfo.positionerModel});
    report.data.push_back({"Отчет ЦТ", 10, 13, valveInfo.solenoidValveModel});
    report.data.push_back({"Отчет ЦТ", 11, 13, valveInfo.limitSwitchModel + "/" + valveInfo.materialStuffingBoxSeal});
    report.data.push_back({"Отчет ЦТ", 12, 14, safeToString(telemetry.supplyPressure)});
    report.data.push_back({"Отчет ЦТ", 13, 14, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 14, 14, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 15, 14, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 16, 14, valveInfo.materialStuffingBoxSeal});

    // Страница: Отчет; Блок: Результат испытаний
    report.data.push_back({"Отчет", 25, 5, safeToString(telemetry.dinamicReal)});
    report.data.push_back({"Отчет", 25, 8, safeToString(telemetry.dinamicRecomend)});

    report.data.push_back({"Отчет", 27, 5, safeToString(telemetry.dinamicIpReal)});
    report.data.push_back({"Отчет", 27, 8, safeToString(telemetry.dinamicIpRecomend)});

    report.data.push_back({"Отчет", 29, 5, safeToString(telemetry.strokeReal)});
    report.data.push_back({"Отчет", 29, 8, safeToString(telemetry.strokeRecomend)});

    report.data.push_back({"Отчет", 31, 5, safeToString(telemetry.rangeReal)});
    report.data.push_back({"Отчет", 31, 8, safeToString(telemetry.rangeRecomend)});

    report.data.push_back({"Отчет", 33, 5, safeToString(telemetry.rangePressure)});

    report.data.push_back({"Отчет", 35, 5, safeToString(telemetry.frictionPercent)});

    report.data.push_back({"Отчет", 37, 5, safeToString(telemetry.friction)});

    report.data.push_back({"Отчет", 51, 5, safeToString(telemetry.strokeTest_timeForward)});
    report.data.push_back({"Отчет", 51, 8, safeToString(telemetry.strokeTest_timeBackward)});

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 65, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 73, 12, otherParams.date});

    // Страница: Отчет; Блок: Диагностические графики клапана, поз.
    report.images.push_back({"Отчет", 85, 1, image1}); // график зависимости ход штока/управляющий сигнал мА
    report.images.push_back({"Отчет", 110, 1, image2}); // график зависимости ход штока/давление в приводе
    report.images.push_back({"Отчет", 135, 1, image3}); // график трения

    // Страница: Отчет; Блок: Дата
    report.data.push_back({"Отчет", 156, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
