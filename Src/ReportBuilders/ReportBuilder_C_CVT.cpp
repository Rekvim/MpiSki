#include "ReportBuilder_C_CVT.h"

ReportBuilder_C_CVT::ReportBuilder_C_CVT() {}

void ReportBuilder_C_CVT::buildReport(
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

    // Страница: Результат теста шаговой реакции; Блок: Данные по объекту
    report.data.push_back({"Результат теста шаговой реакции", 4, 4, objectInfo.object});
    report.data.push_back({"Результат теста шаговой реакции", 5, 4, objectInfo.manufactory});
    report.data.push_back({"Результат теста шаговой реакции", 6, 4, objectInfo.department});

    // Страница:Результат теста шаговой реакции; Блок: Краткая спецификация на клапан
    report.data.push_back({"Результат теста шаговой реакции", 4, 13, valveInfo.positionNumber});
    report.data.push_back({"Результат теста шаговой реакции", 5, 13, valveInfo.serialNumber});
    report.data.push_back({"Результат теста шаговой реакции", 6, 13, valveInfo.valveModel});
    report.data.push_back({"Результат теста шаговой реакции", 7, 13, valveInfo.manufacturer});
    report.data.push_back({"Результат теста шаговой реакции", 8, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Результат теста шаговой реакции", 9, 13, valveInfo.positionerModel});
    report.data.push_back({"Результат теста шаговой реакции", 10, 13, valveInfo.solenoidValveModel});
    report.data.push_back({"Результат теста шаговой реакции", 11, 13, valveInfo.limitSwitchModel + "/" + valveInfo.positionSensorModel});
    report.data.push_back({"Результат теста шаговой реакции", 12, 13, telemetryStore.supplyRecord.supplyPressure});
    report.data.push_back({"Результат теста шаговой реакции", 13, 13, otherParams.safePosition});
    report.data.push_back({"Результат теста шаговой реакции", 14, 13, valveInfo.driveModel});
    report.data.push_back({"Результат теста шаговой реакции", 15, 13, otherParams.strokeMovement});
    report.data.push_back({"Результат теста шаговой реакции", 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    quint16 excelRow = 23;
    for (auto &r : telemetryStore.stepResults) {
        report.data.push_back({ "Результат теста шаговой реакции", excelRow, 3, r.range });
        report.data.push_back({ "Результат теста шаговой реакции", excelRow, 4, r.T86sec });
        report.data.push_back({ "Результат теста шаговой реакции", excelRow, 5, r.overshoot + "%" });
        ++excelRow;
    }

    report.data.push_back({"Результат теста шаговой реакции", 44, 12, otherParams.date});


    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 60, 12, otherParams.date});

    // Страница: Отчет; Блок: Данные по объекту
    report.data.push_back({"Отчет", 4, 4, objectInfo.object});
    report.data.push_back({"Отчет", 5, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет", 6, 4, objectInfo.department});

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    report.data.push_back({"Отчет", 4, 13, valveInfo.positionNumber});
    report.data.push_back({"Отчет", 5, 13, valveInfo.serialNumber});
    report.data.push_back({"Отчет", 6, 13, valveInfo.valveModel});
    report.data.push_back({"Отчет", 7, 13, valveInfo.manufacturer});
    report.data.push_back({"Отчет", 8, 13, valveInfo.DN + "/" + valveInfo.PN});
    report.data.push_back({"Отчет", 9, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет", 10, 13, telemetryStore.supplyRecord.supplyPressure});
    report.data.push_back({"Отчет", 11, 13, safeToString(valveInfo.safePosition)});
    report.data.push_back({"Отчет", 12, 13, safeToString(valveInfo.strokeMovement)});
    report.data.push_back({"Отчет", 13, 13, valveInfo.positionerModel});
    report.data.push_back({"Отчет", 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница: Отчет; Блок: Результат испытаний
    report.data.push_back({"Отчет", 20, 5, telemetryStore.dinamicRecord.dinamicReal});
    report.data.push_back({"Отчет", 20, 8, telemetryStore.dinamicRecord.dinamicRecomend});

    report.data.push_back({"Отчет", 22, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    report.data.push_back({"Отчет", 22, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    report.data.push_back({"Отчет", 24, 5, telemetryStore.strokeRecord.strokeReal});
    report.data.push_back({"Отчет", 24, 8, telemetryStore.strokeRecord.strokeRecomend});

    report.data.push_back({"Отчет", 26, 5, telemetryStore.rangeRecord.rangeReal});
    report.data.push_back({"Отчет", 26, 8, telemetryStore.rangeRecord.rangeRecomend});

    report.data.push_back({"Отчет", 28, 5, telemetryStore.rangeRecord.rangePressure});

    report.data.push_back({"Отчет", 30, 5, telemetryStore.frictionRecord.frictionPercent});

    report.data.push_back({"Отчет", 32, 5, telemetryStore.frictionRecord.friction});

    report.data.push_back({"Отчет", 46, 5, telemetryStore.strokeTestRecord.timeForward});
    report.data.push_back({"Отчет", 46, 8, telemetryStore.strokeTestRecord.timeBackward});

    // Страница: Отчет; Блок: Дата
    report.data.push_back({"Отчет", 60, 12, otherParams.date});

    // Страница: Отчет; Блок: Исполнитель
    report.data.push_back({"Отчет", 68, 4, objectInfo.FIO});

    // Страница: Отчет; Блок: Диагностические графики клапана, поз.
    report.images.push_back({"Отчет", 80, 1, image1}); // график зависимости ход штока/управляющий сигнал мА
    report.images.push_back({"Отчет", 105, 1, image2}); // график зависимости ход штока/давление в приводе
    report.images.push_back({"Отчет", 130, 1, image3}); // график трения

    // Страница: Отчет; Блок: Дата
    report.data.push_back({"Отчет", 156, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
