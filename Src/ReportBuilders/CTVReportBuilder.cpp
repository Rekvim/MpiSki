#include "CTVReportBuilder.h"

CTVReportBuilder::CTVReportBuilder() {}

void CTVReportBuilder::buildReport(
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
    // report.data.push_back({"Отчет ЦТ", 9, 13, ???}); // Соленоидный клапан
    // report.data.push_back({"Отчет ЦТ", 10, 13, ???}); // Концевой выключатель/датчик положения
    report.data.push_back({"Отчет ЦТ", 11, 13, safeToString(telemetry.supplyPressure)});
    report.data.push_back({"Отчет ЦТ", 12, 13, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 13, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 14, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 15, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    report.data.push_back({"Отчет ЦТ", 20, 8, safeToString(telemetry.strokeTest_timeForward)}); // Результат теста полного хода
    report.data.push_back({"Отчет ЦТ", 22, 8, safeToString(telemetry.strokeTest_timeBackward)}); // Результат теста полного хода
    // report.data.push_back({"Отчет ЦТ", 24, 8, safeToString(???)});
    // report.data.push_back({"Отчет ЦТ", 26, 8, safeToString(telemetry.cyclicTest_rangePercent)}); // Указывается выбранное значение ЦТ (по умолчанию либо введеный вручную)
    // report.data.push_back({"Отчет ЦТ", 28, 8, safeToString(telemetry.cyclicTest_totalTime)});

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    // report.data.push_back({"Отчет ЦТ", 36, 8, safeToString(???)}); // Задание диапазона (0% хода)
    // report.data.push_back({"Отчет ЦТ", 38, 8, safeToString(???)}); // Задание диапазона (100% хода)

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    // report.data.push_back({"Отчет ЦТ", 44, 8, valveInfo.???}); // Положение "О-3"
    // report.data.push_back({"Отчет ЦТ", 46, 8, valveInfo.???}); // Положение "З-О"

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 53, 4, objectInfo.FIO});

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 57, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
