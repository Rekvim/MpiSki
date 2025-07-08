#include "ReportBuilder_B_SACVT.h"

ReportBuilder_B_SACVT::ReportBuilder_B_SACVT() {}

void ReportBuilder_B_SACVT::buildReport(
    ReportSaver::Report& report,
    const TelemetryStore& telemetryStore,
    const ObjectInfo& objectInfo,
    const ValveInfo& valveInfo,
    const OtherParameters& otherParams,
    const QImage& imageChartTask,
    const QImage& imageChartPressure,
    const QImage& imageChartFriction,
    const QImage& imageChartStep
    )
{
    QString sheet_1 = "Отчет ЦТ";

    // Лист 1; Страница: Отчет ЦТ; Блок: Данные по объекту
    report.data.push_back({sheet_1, 4, 4, objectInfo.object});
    report.data.push_back({sheet_1, 5, 4, objectInfo.manufactory});
    report.data.push_back({sheet_1, 6, 4, objectInfo.department});

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    report.data.push_back({sheet_1, 4, 13, valveInfo.positionNumber});
    report.data.push_back({sheet_1, 5, 13, valveInfo.serialNumber});
    report.data.push_back({sheet_1, 6, 13, valveInfo.valveModel});
    report.data.push_back({sheet_1, 7, 13, valveInfo.manufacturer});
    report.data.push_back({sheet_1, 8, 13, QString("%1 / %2")
                                               .arg(valveInfo.DN)
                                               .arg(valveInfo.PN)});
    report.data.push_back({sheet_1, 9, 13, valveInfo.positionerModel});
    report.data.push_back({sheet_1, 10, 13, valveInfo.solenoidValveModel});
    report.data.push_back({sheet_1, 11, 13, valveInfo.limitSwitchModel + "/" + valveInfo.positionSensorModel});
    report.data.push_back({sheet_1, 12, 13, safeToString(telemetryStore.supplyRecord.pressure_bar)});
    report.data.push_back({sheet_1, 13, 13, otherParams.safePosition});
    report.data.push_back({sheet_1, 14, 13, valveInfo.driveModel});
    report.data.push_back({sheet_1, 15, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_1, 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({sheet_1, 21, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeForwardMs)
                                               .toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 23, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs)
                                               .toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 25, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 27, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 29, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec)
                                               .toString("mm:ss.zzz")});

    // // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
    {
        const auto& ranges = telemetryStore.cyclicTestRecord.ranges;
        constexpr quint16 rowStart = 35, rowStep = 2;
        for (int i = 0; i < qMin(ranges.size(), 10); ++i) {
            quint16 row = rowStart + i * rowStep;
            report.data.push_back({sheet_1, row,  2,
                                   QString::number(ranges[i].rangePercent)});
            report.data.push_back({sheet_1, row,  8,
                QString("%1 %/ № %2")
                    .arg(ranges[i].maxForwardValue, 0, 'f', 2)
                    .arg(ranges[i].maxForwardCycle)
            });
            report.data.push_back({sheet_1, row, 10,
                QString("%1 %/ №%2")
                    .arg(ranges[i].maxReverseValue, 0, 'f', 2)
                    .arg(ranges[i].maxReverseCycle)
            });
        }
    }

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_1, 58, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_1, 62, 12, otherParams.date});

    // Лист 2; Страница: Отчет ЦТ; Блок: Данные по объекту
    report.data.push_back({sheet_1, 68, 4, objectInfo.object});
    report.data.push_back({sheet_1, 69, 4, objectInfo.manufactory});
    report.data.push_back({sheet_1, 70, 4, objectInfo.department});

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    report.data.push_back({sheet_1, 68, 13, valveInfo.positionNumber});
    report.data.push_back({sheet_1, 69, 13, valveInfo.serialNumber});
    report.data.push_back({sheet_1, 70, 13, valveInfo.valveModel});
    report.data.push_back({sheet_1, 71, 13, valveInfo.manufacturer});
    report.data.push_back({sheet_1, 72, 13, QString("%1 / %2")
                                                .arg(valveInfo.DN)
                                                .arg(valveInfo.PN)});
    report.data.push_back({sheet_1, 73, 13, valveInfo.positionerModel});
    report.data.push_back({sheet_1, 74, 13, valveInfo.solenoidValveModel});
    report.data.push_back({sheet_1, 75, 13, QString("%1 / %2")
                                                .arg(valveInfo.limitSwitchModel)
                                                .arg(valveInfo.positionSensorModel)});
    report.data.push_back({sheet_1, 76, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});
    report.data.push_back({sheet_1, 77, 13, otherParams.safePosition});
    report.data.push_back({sheet_1, 78, 13, valveInfo.driveModel});
    report.data.push_back({sheet_1, 79, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_1, 80, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    report.data.push_back({sheet_1, 85, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeForwardMs)
                                               .toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 87, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs)
                                               .toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 89, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 91, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 93, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec)
                                               .toString("mm:ss.zzz")});

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    // Задание диапазона (0% хода)
    report.data.push_back({sheet_1, 101, 8, safeToString(telemetryStore.cyclicTestRecord.cycles)}); // Количество циклов
    // report.data.push_back({sheet_1, 101, 10, safeToString(???)}); // Количество включений
    // report.data.push_back({sheet_1, 101, 13, safeToString(???)}); // Количество выключений
    // Задание диапазона (100% хода)
    report.data.push_back({sheet_1, 103, 8, safeToString(telemetryStore.cyclicTestRecord.cycles)}); // Количество циклов
    // report.data.push_back({sheet_1, 103, 10, safeToString(???)}); // Количество включений
    // report.data.push_back({sheet_1, 103, 13, safeToString(???)}); // Количество выключений

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    // Положение 0-3
    report.data.push_back({sheet_1, 109, 8, safeToString(telemetryStore.cyclicTestRecord.cycles)});  // Количество циклов
    // report.data.push_back({sheet_1, 109, 10, safeToString(telemetryStore.cyclicTestRecord.)}); // Количество срабатываний
    // report.data.push_back({sheet_1, 109, 13, safeToString(telemetryStore.cyclicTestRecord.)}); // Количество ошибок на срабатыввание

    // Положение 3-0
    report.data.push_back({sheet_1, 111, 8, safeToString(telemetryStore.cyclicTestRecord.cycles)}); // Количество циклов
    // report.data.push_back({sheet_1, 111, 10, safeToString(telemetryStore.cyclicTestRecord.)}); // Количество срабатываний
    // report.data.push_back({sheet_1, 111, 13, safeToString(telemetryStore.cyclicTestRecord.)}); // Количество ошибок на срабатыввание

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_1, 118, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_1, 122, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
