#include "ReportBuilder_C_SACVT.h"

ReportBuilder_C_SACVT::ReportBuilder_C_SACVT() {}

void ReportBuilder_C_SACVT::buildReport(
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
    report.data.push_back({sheet_1, 11, 13, QString("%1 / %2")
                                                .arg(valveInfo.limitSwitchModel)
                                                .arg(valveInfo.positionSensorModel)});
    report.data.push_back({sheet_1, 12, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});
    report.data.push_back({sheet_1, 13, 13, otherParams.safePosition});
    report.data.push_back({sheet_1, 14, 13, valveInfo.driveModel});
    report.data.push_back({sheet_1, 15, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_1, 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({sheet_1, 21, 8, telemetryStore.strokeTestRecord.timeForwardMs});
    report.data.push_back({sheet_1, 23, 8, telemetryStore.strokeTestRecord.timeBackwardMs});
    report.data.push_back({sheet_1, 25, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 27, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 29, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec).toString("mm:ss.zzz")});


    // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
    {
        const auto& ranges = telemetryStore.cyclicTestRecord.ranges;

        struct Agg {
            qreal rangePercent;
            qreal maxFwdVal = std::numeric_limits<qreal>::lowest();
            int maxFwdCycle = -1;
            qreal minRevVal = std::numeric_limits<qreal>::max();
            int minRevCycle = -1;
        };
        QMap<qreal, Agg> aggMap;

        // 1) Собираем все rec по ключу rangePercent
        for (const auto& rec : ranges) {
            auto it = aggMap.find(rec.rangePercent);
            if (it == aggMap.end()) {
                // первый раз для этого percent
                Agg a;
                a.rangePercent = rec.rangePercent;
                // прямой ход
                if (rec.maxForwardCycle >= 0) {
                    a.maxFwdVal   = rec.maxForwardValue;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход
                if (rec.maxReverseCycle >= 0) {
                    a.minRevVal   = rec.maxReverseValue;
                    a.minRevCycle = rec.maxReverseCycle;
                }
                aggMap.insert(rec.rangePercent, a);

            } else {
                // уже есть — обновляем экстремумы
                Agg &a = it.value();
                // прямой ход — берём глобальный максимум
                if (rec.maxForwardCycle >= 0
                    && rec.maxForwardValue > a.maxFwdVal) {
                    a.maxFwdVal   = rec.maxForwardValue;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход — берём глобальный минимум
                if (rec.maxReverseCycle >= 0
                    && rec.maxReverseValue < a.minRevVal) {
                    a.minRevVal   = rec.maxReverseValue;
                    a.minRevCycle = rec.maxReverseCycle;
                }
            }
        }

        // 2) Выводим первые 10 (или сколько есть) диапазонов
        QVector<qreal> percents;
        percents.reserve(aggMap.size());
        for (auto it = aggMap.constBegin(); it != aggMap.constEnd(); ++it) {
            percents.append(it.key());
        }

        constexpr quint16 rowStart = 35, rowStep = 2;
        for (int i = 0; i < percents.size() && i < 10; ++i) {
            quint16 row = rowStart + i * rowStep;
            const Agg &a = aggMap[percents[i]];

            // Процент
            report.data.push_back({
                sheet_1, row, 2,
                QString::number(a.rangePercent)
            });

            // Прямой ход (максимум)
            if (a.maxFwdCycle >= 0) {
                report.data.push_back({
                    sheet_1, row, 8,
                    QString("%1 %")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                });
                report.data.push_back({
                    sheet_1, row, 9,
                    QString("№ %1")
                        .arg(a.maxFwdCycle + 1)
                });
            } else {
                // нет данных
                report.data.push_back({ sheet_1, row, 8, QString() });
                report.data.push_back({ sheet_1, row, 9, QString() });
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                report.data.push_back({
                    sheet_1, row, 10,
                    QString("%1 %")
                        .arg(a.minRevVal,   0, 'f', 2)
                });
                report.data.push_back({
                    sheet_1, row, 12,
                    QString("№ %1")
                        .arg(a.minRevCycle + 1)
                });
            } else {
                report.data.push_back({ sheet_1, row, 10, QString() });
                report.data.push_back({ sheet_1, row, 12, QString() });
            }
        }
    }

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
    report.data.push_back({sheet_1, 85, 8, telemetryStore.strokeTestRecord.timeForwardMs});
    report.data.push_back({sheet_1, 87, 8, telemetryStore.strokeTestRecord.timeBackwardMs});
    report.data.push_back({sheet_1, 89, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 91, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 93, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec)
                                               .toString("mm:ss.zzz")});

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    const auto &ons  = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto &offs = telemetryStore.cyclicTestRecord.doOffCounts;
    for (int i = 0; i < ons.size(); ++i) {
        if (ons[i] == 0 && offs.value(i, 0) == 0)
            continue;

        quint16 row = 101 + quint16(i) * 2;
        report.data.push_back({
            "Отчет ЦТ", row, 10,
            QString::number(ons[i])
        });
        report.data.push_back({
            "Отчет ЦТ", row, 13,
            QString::number(offs.value(i, 0))
        });
    }

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    report.data.push_back({
        "Отчет ЦТ", 109, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 109, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 109, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 111, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 111, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 111, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    });

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_1, 118, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_1, 122, 12, otherParams.date});

    // Страница: Результат теста шаговой реакции

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
    report.data.push_back({sheet_2, 10, 13, valveInfo.solenoidValveModel});
    report.data.push_back({sheet_2, 11, 13, QString("%1 / %2")
                                                .arg(valveInfo.limitSwitchModel)
                                                .arg(valveInfo.positionSensorModel)});
    report.data.push_back({sheet_2, 12, 13, QString::asprintf("%.2f bar", telemetryStore.supplyRecord.pressure_bar)});
    report.data.push_back({sheet_2, 13, 13, otherParams.safePosition});
    report.data.push_back({sheet_2, 14, 13, valveInfo.driveModel});
    report.data.push_back({sheet_2, 15, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_2, 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({sheet_2, 20, 2, imageChartStep}); // график зависимости ход штока/управляющий сигнал мА

    // Страница:Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    quint16 row = 57;
    for (auto &sr : telemetryStore.stepResults) {
        report.data.push_back({
            sheet_2, row, 3, QString("%1–%2").arg(sr.from).arg(sr.to)
        });
        report.data.push_back({
            sheet_2, row, 4, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz")
        });
        report.data.push_back({
            sheet_2, row, 5,
            QString("%1").arg(sr.overshoot, 0, 'f', 2)
        });
        ++row;
    }

    report.data.push_back({sheet_2, 78, 12, otherParams.date});

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
    report.data.push_back({sheet_3, 10, 13, valveInfo.solenoidValveModel});
    report.data.push_back({sheet_3, 11, 13, QString("%1 / %2")
                                                .arg(valveInfo.limitSwitchModel)
                                                .arg(valveInfo.positionSensorModel)});
    report.data.push_back({sheet_3,12, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});
    report.data.push_back({sheet_3, 13, 13, otherParams.safePosition});
    report.data.push_back({sheet_3, 14, 13, valveInfo.driveModel});
    report.data.push_back({sheet_3, 15, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_3, 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница: Отчет; Блок: Результат испытаний
    report.data.push_back({sheet_3, 22, 5,
                           QString("%1 %")
                                .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2)});

    report.data.push_back({sheet_3, 22, 8,
                           QString("%1 %")
                                .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2)});

    // report.data.push_back({sheet_3, 24, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // report.data.push_back({sheet_3, 24, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    report.data.push_back({sheet_3, 26, 5,
                           QString("%1")
                                .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2)});
    report.data.push_back({sheet_3, 26, 8, valveInfo.strokValve});

    report.data.push_back({
        sheet_3, 28, 5,
        QString("%1—%2")
            .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
    });
    report.data.push_back({sheet_3, 28, 8, valveInfo.driveRecomendRange});

    report.data.push_back({sheet_3, 30, 5,
        QString("%1—%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    });

    report.data.push_back({ sheet_3, 32, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    });

    report.data.push_back({ sheet_3, 34, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    });
    report.data.push_back({ sheet_3, 48, 5,telemetryStore.strokeTestRecord.timeForwardMs
    });
    report.data.push_back({
        sheet_3, 48, 8,telemetryStore.strokeTestRecord.timeBackwardMs
    });

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_3, 62, 12, otherParams.date});
    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_3, 70, 4, objectInfo.FIO});

    // Страница: Отчет; Блок: Диагностические графики клапана, поз.
    report.images.push_back({sheet_3, 82, 1, imageChartTask}); // график зависимости ход штока/управляющий сигнал мА
    report.images.push_back({sheet_3, 106, 1, imageChartPressure}); // график зависимости ход штока/давление в приводе
    report.images.push_back({sheet_3, 132, 1, imageChartFriction}); // график трения

    // Страница: Отчет; Блок: Дата
    report.data.push_back({sheet_3, 155, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
