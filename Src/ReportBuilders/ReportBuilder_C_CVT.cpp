#include "ReportBuilder_C_CVT.h"

ReportBuilder_C_CVT::ReportBuilder_C_CVT() {}

void ReportBuilder_C_CVT::buildReport(
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
    QString sheet_2 = "Результат теста шаговой реакции";
    QString sheet_3 = "Отчет";

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
    report.data.push_back({sheet_1, 19, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeForwardMs)
                                                     .toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 21, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs)
                                                     .toString("mm:ss.zzz")});
    report.data.push_back({sheet_1, 23, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 25, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 27, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec)
                                                     .toString("mm:ss.zzz")});

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
                QString("%1 %/ № %2")
                    .arg(ranges[i].maxReverseValue, 0, 'f', 2)
                    .arg(ranges[i].maxReverseCycle)
            });
        }
    }

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_1, 56, 4, objectInfo.FIO});
    report.data.push_back({sheet_1, 56, 4, otherParams.date});

    // Страница: Результат теста шаговой реакции; Блок: Данные по объекту
    report.data.push_back({sheet_2, 4, 4, objectInfo.object});
    report.data.push_back({sheet_2, 5, 4, objectInfo.manufactory});
    report.data.push_back({sheet_2, 6, 4, objectInfo.department});

    // Страница:Результат теста шаговой реакции; Блок: Краткая спецификация на клапан
    report.data.push_back({sheet_2, 4, 13, valveInfo.positionNumber});
    report.data.push_back({sheet_2, 5, 13, valveInfo.serialNumber});
    report.data.push_back({sheet_2, 6, 13, valveInfo.valveModel});
    report.data.push_back({sheet_2, 7, 13, valveInfo.manufacturer});
    report.data.push_back({sheet_2, 8, 13, QString("%1 / %2")
                                               .arg(valveInfo.DN)
                                               .arg(valveInfo.PN)});
    report.data.push_back({sheet_2, 9, 13, valveInfo.positionerModel});
    report.data.push_back({sheet_2, 10, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});    
    report.data.push_back({sheet_2, 11, 13, otherParams.safePosition});
    report.data.push_back({sheet_2, 12, 13, valveInfo.driveModel});
    report.data.push_back({sheet_2, 13, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_2, 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({sheet_2, 18, 2, imageChartStep});

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    {
        quint16 row = 55;
        for (auto &sr : telemetryStore.stepResults) {
            report.data.push_back({sheet_2, row, 3, QString("%1->%2").arg(sr.from).arg(sr.to)});
            report.data.push_back({sheet_2, row, 4, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz")});
            report.data.push_back({sheet_2, row, 5, QString("%1").arg(sr.overshoot, 0, 'f', 2)
            });
            ++row;
        }
    }

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_2, 76, 12, otherParams.date});

    // Страница: Отчет; Блок: Данные по объекту
    report.data.push_back({sheet_3, 4, 4, objectInfo.object});
    report.data.push_back({sheet_3, 5, 4, objectInfo.manufactory});
    report.data.push_back({sheet_3, 6, 4, objectInfo.department});

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    report.data.push_back({sheet_3, 4, 13, valveInfo.positionNumber});
    report.data.push_back({sheet_3, 5, 13, valveInfo.serialNumber});
    report.data.push_back({sheet_3, 6, 13, valveInfo.valveModel});
    report.data.push_back({sheet_3, 7, 13, valveInfo.manufacturer});
    report.data.push_back({sheet_3, 8, 13, QString("%1 / %2")
                                               .arg(valveInfo.DN)
                                               .arg(valveInfo.PN)});
    report.data.push_back({sheet_3, 9, 13, valveInfo.positionerModel});
    report.data.push_back({sheet_3, 10, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});    
    report.data.push_back({sheet_3, 11, 13, otherParams.safePosition});
    report.data.push_back({sheet_3, 12, 13, valveInfo.driveModel});
    report.data.push_back({sheet_3, 13, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_3, 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница: Отчет; Блок: Результат испытаний
    report.data.push_back({sheet_3, 20, 5,
                            QString("%1 %")
                               .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2)});

    report.data.push_back({sheet_3, 20, 8,
                           QString("%1 %")
                                .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2)});

    // report.data.push_back({sheet_3, 22, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // report.data.push_back({sheet_3, 22, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    report.data.push_back({sheet_3, 24, 5, QString("%1")
                                               .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2)});
    report.data.push_back({sheet_3, 24, 8, valveInfo.strokValve});

    report.data.push_back({sheet_3, 26, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
    });
    report.data.push_back({sheet_3, 26, 8, valveInfo.driveRecomendRange});

    report.data.push_back({sheet_3, 28, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    });

    report.data.push_back({
        sheet_3, 30, 5, 
        QString("%1")
           .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    });

    report.data.push_back({
        sheet_3, 32, 5,
        QString("%1")
           .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    });
    report.data.push_back({
        sheet_3, 46, 5,
        QTime(0,0)
        .addMSecs(telemetryStore.strokeTestRecord.timeForwardMs)
        .toString("mm:ss.zzz")
    });
    report.data.push_back({
        sheet_3, 46, 8,
        QTime(0,0)
        .addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs)
        .toString("mm:ss.zzz")
    });

    // Дата и Исполнитель
    report.data.push_back({sheet_3, 60, 12, otherParams.date});
    report.data.push_back({sheet_3, 68,  4, objectInfo.FIO});

    // ------------------------------------------------------------------------
    // Страница: Отчет; Блок: Диагностические графики
    // ------------------------------------------------------------------------
    report.images.push_back({sheet_3,  80, 1, imageChartTask});
    report.images.push_back({sheet_3, 105, 1, imageChartPressure});
    report.images.push_back({sheet_3, 130, 1, imageChartFriction});

    // Страница: Отчет; Блок: Дата
    report.data.push_back({sheet_3, 156, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
