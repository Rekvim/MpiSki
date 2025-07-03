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
    report.data.push_back({ "Отчет ЦТ", 10, 13, QString::asprintf("%.2f", telemetryStore.supplyRecord.pressure_bar) });
    report.data.push_back({"Отчет ЦТ", 11, 13, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 12, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 13, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({"Отчет ЦТ", 19, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeForwardMs).toString("mm:ss.zzz")});
    report.data.push_back({"Отчет ЦТ", 21, 8, QTime(0,0).addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs).toString("mm:ss.zzz")});
    report.data.push_back({"Отчет ЦТ", 23, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({"Отчет ЦТ", 25, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({"Отчет ЦТ", 27, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec).toString("mm:ss.zzz")});

    {
        const QString sheet = QStringLiteral("Отчет ЦТ");
        const auto& ranges = telemetryStore.cyclicTestRecord.ranges;
        constexpr quint16 rowStart = 33, rowStep = 2;
        for (int i = 0; i < qMin(ranges.size(), 10); ++i) {
            quint16 row = rowStart + i * rowStep;
            const auto& r = ranges[i];
            report.data.push_back({ sheet, row,  2, QString::number(r.rangePercent) });
            report.data.push_back({ sheet, row,  8,
                QString::number(r.maxForwardValue, 'f', 2)
                    + "/" +
                    QString::number(r.maxForwardCycle)
            });
            report.data.push_back({ sheet, row, 10,
                QString::number(r.maxReverseValue, 'f', 2)
                    + "/" +
                    QString::number(r.maxReverseCycle)
            });
        }
    }

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
    report.data.push_back({"Результат теста шаговой реакции", 10, 13, QString::asprintf("%.2f", telemetryStore.supplyRecord.pressure_bar) });
    report.data.push_back({"Результат теста шаговой реакции", 11, 13, otherParams.safePosition});
    report.data.push_back({"Результат теста шаговой реакции", 12, 13, valveInfo.driveModel});
    report.data.push_back({"Результат теста шаговой реакции", 13, 13, otherParams.strokeMovement});
    report.data.push_back({"Результат теста шаговой реакции", 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({"Результат теста шаговой реакции", 18, 2, imageChartStep}); // график зависимости ход штока/управляющий сигнал мА

    // Страница:Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    {
        quint16 row = 55;
        for (auto &sr : telemetryStore.stepResults) {
            report.data.push_back({
                "Результат теста шаговой реакции", row, 3, sr.range
            });
            report.data.push_back({
                "Результат теста шаговой реакции", row, 4,
                QTime(0,0).addMSecs(sr.T_ms).toString("m:ss.zzz")
            });
            report.data.push_back({
                "Результат теста шаговой реакции", row, 5,
                QString::asprintf("%.2f %%", sr.overshootPct)
            });
            ++row;
        }
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
    report.data.push_back({"Отчет", 10, 13, QString::asprintf("%.2f", telemetryStore.supplyRecord.pressure_bar) });
    report.data.push_back({"Отчет", 11, 13, otherParams.safePosition});
    report.data.push_back({"Отчет", 12, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет", 13, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет", 14, 13, valveInfo.materialStuffingBoxSeal});

    // Страница: Отчет; Блок: Результат испытаний
    report.data.push_back({
        "Отчет", 20, 5,
        QString::asprintf("%.2f %%", telemetryStore.mainTestRecord.dynamicReal)
    });

    report.data.push_back({"Отчет", 20, 8, safeToString(valveInfo.dinamicErrorRecomend)});

    // report.data.push_back({"Отчет", 22, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // report.data.push_back({"Отчет", 22, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    report.data.push_back({"Отчет", 24, 5, safeToString(telemetryStore.strokeRecord.strokeReal)});
    report.data.push_back({"Отчет", 24, 8, safeToString(telemetryStore.strokeRecord.strokeRecomend)});

    report.data.push_back({
        "Отчет", 26, 5,
        QString::asprintf("%.2f – %.2f",
            telemetryStore.mainTestRecord.springLow,
            telemetryStore.mainTestRecord.springHigh
          )
    });

    report.data.push_back({"Отчет", 26, 8, valveInfo.driveRecomendRange});

    report.data.push_back({"Отчет", 28, 5,
                           QString::asprintf("%.2f – %.2f",
                                             telemetryStore.mainTestRecord.lowLimit,
                                             telemetryStore.mainTestRecord.highLimit
                                             )});



    report.data.push_back({ "Отчет", 30, 5,
        QString::asprintf("%.2f %%", telemetryStore.mainTestRecord.frictionPercent)
    });

    report.data.push_back({ "Отчет", 32, 5,
        QString::asprintf("%.3f", telemetryStore.mainTestRecord.frictionForce)
    });
    report.data.push_back({ "Отчет", 46, 5,
        QTime(0,0)
            .addMSecs(telemetryStore.strokeTestRecord.timeForwardMs)
            .toString("mm:ss.zzz")
    });
    report.data.push_back({
        "Отчет", 46, 8,
        QTime(0,0)
            .addMSecs(telemetryStore.strokeTestRecord.timeBackwardMs)
            .toString("mm:ss.zzz")
    });

    // Дата и Исполнитель
    report.data.push_back({"Отчет", 60, 12, otherParams.date});
    report.data.push_back({"Отчет", 68,  4, objectInfo.FIO});

    // ------------------------------------------------------------------------
    // Страница: Отчет; Блок: Диагностические графики
    // ------------------------------------------------------------------------
    report.images.push_back({"Отчет",  80, 1, imageChartTask});
    report.images.push_back({"Отчет", 105, 1, imageChartPressure});
    report.images.push_back({"Отчет", 130, 1, imageChartFriction});

    // Страница: Отчет; Блок: Дата
    report.data.push_back({"Отчет", 156, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
