#include "ReportBuilder_B_CVT.h"

ReportBuilder_B_CVT::ReportBuilder_B_CVT() {}

void ReportBuilder_B_CVT::buildReport(
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

    // Страница: Отчет ЦТ; Блок: Данные по объекту
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
    report.data.push_back({sheet_1, 10, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});
    report.data.push_back({sheet_1, 11, 13, otherParams.safePosition});
    report.data.push_back({sheet_1, 12, 13, valveInfo.driveModel});
    report.data.push_back({sheet_1, 13, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_1, 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({sheet_1, 19, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeForwardMs).toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 21, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs).toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 23, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 25, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 27, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec).toString("mm:ss.zzz")});

    // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
    {
        const auto& ranges = telemetryStore.cyclicTestRecord.ranges;
        constexpr quint16 rowStart = 33, rowStep = 2;
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
    report.data.push_back({sheet_1, 56, 4, objectInfo.FIO});

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_1, 60, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
