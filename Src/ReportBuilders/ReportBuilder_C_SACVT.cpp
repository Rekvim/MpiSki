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

    // Лист: Отчет ЦТ; Страница: 1; Блок: Данные по объекту
    cell(report, sheet_1, 4, 4, objectInfo.object);
    cell(report, sheet_1, 5, 4, objectInfo.manufactory);
    cell(report, sheet_1, 6, 4, objectInfo.department);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Краткая спецификация на клапан
    cell(report, sheet_1, 4, 13, valveInfo.positionNumber);
    cell(report, sheet_1, 5, 13, valveInfo.serialNumber);
    cell(report, sheet_1, 6, 13, valveInfo.valveModel);
    cell(report, sheet_1, 7, 13, valveInfo.manufacturer);
    cell(report, sheet_1, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_1, 9, 13, valveInfo.positionerModel);
    cell(report, sheet_1, 10, 13, valveInfo.solenoidValveModel);
    cell(report, sheet_1, 11, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, sheet_1, 12, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_1, 13, 13, otherParams.safePosition);
    cell(report, sheet_1, 14, 13, valveInfo.driveModel);
    cell(report, sheet_1, 15, 13, otherParams.strokeMovement);
    cell(report, sheet_1, 16, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Результат испытаний позиционера
    cell(report, sheet_1, 21, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, sheet_1, 23, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, sheet_1, 25, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, sheet_1, 27, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, sheet_1, 29, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory).toString("mm:ss.zzz"));


    // Лист: Отчет ЦТ; Страница: 1 Блок: Циклические испытания позиционера
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
            cell(report,
                sheet_1, row, 2,
                QString::number(a.rangePercent)
            );

            // Прямой ход (максимум)
            if (a.maxFwdCycle >= 0) {
                cell(report,
                    sheet_1, row, 8,
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                );
                cell(report,
                    sheet_1, row, 11,
                    QString("%1")
                        .arg(a.maxFwdCycle + 1)
                );
            } else {
                // нет данных
                cell(report, sheet_1, row, 8, QString());
                cell(report, sheet_1, row, 11, QString());
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                cell(report,
                    sheet_1, row, 12,
                    QString("%1")
                        .arg(a.minRevVal,   0, 'f', 2)
                );
                cell(report,
                    sheet_1, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                );
            } else {
                cell(report, sheet_1, row, 12, QString());
                cell(report, sheet_1, row, 15, QString());
            }
        }
    }

    // Лист 2; Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, sheet_1, 68, 4, objectInfo.object);
    cell(report, sheet_1, 69, 4, objectInfo.manufactory);
    cell(report, sheet_1, 70, 4, objectInfo.department);

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, sheet_1, 68, 13, valveInfo.positionNumber);
    cell(report, sheet_1, 69, 13, valveInfo.serialNumber);
    cell(report, sheet_1, 70, 13, valveInfo.valveModel);
    cell(report, sheet_1, 71, 13, valveInfo.manufacturer);
    cell(report, sheet_1, 72, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_1, 73, 13, valveInfo.positionerModel);
    cell(report, sheet_1, 74, 13, valveInfo.solenoidValveModel);
    cell(report, sheet_1, 75, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, sheet_1, 76, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_1, 77, 13, otherParams.safePosition);
    cell(report, sheet_1, 78, 13, valveInfo.driveModel);
    cell(report, sheet_1, 79, 13, otherParams.strokeMovement);
    cell(report, sheet_1, 80, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Результат испытаний позиционера
    cell(report, sheet_1, 85, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, sheet_1, 87, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, sheet_1, 89, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, sheet_1, 91, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, sheet_1, 93, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory).toString("mm:ss.zzz"));


    // Лист 2; Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, sheet_1, 122, 4, objectInfo.FIO);
    // Лист 2; Страница: Отчет ЦТ; Блок: Дата
    cell(report, sheet_1, 126, 12, otherParams.date);


    // Лист 3; Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, sheet_1, 131, 4, objectInfo.object);
    cell(report, sheet_1, 132, 4, objectInfo.manufactory);
    cell(report, sheet_1, 133, 4, objectInfo.department);

    // Лист 3; Страница: Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, sheet_1, 131, 13, valveInfo.positionNumber);
    cell(report, sheet_1, 132, 13, valveInfo.serialNumber);
    cell(report, sheet_1, 133, 13, valveInfo.valveModel);
    cell(report, sheet_1, 134, 13, valveInfo.manufacturer);
    cell(report, sheet_1, 135, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_1, 136, 13, valveInfo.positionerModel);
    cell(report, sheet_1, 137, 13, valveInfo.solenoidValveModel);
    cell(report, sheet_1, 138, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, sheet_1, 139, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_1, 140, 13, otherParams.safePosition);
    cell(report, sheet_1, 141, 13, valveInfo.driveModel);
    cell(report, sheet_1, 142, 13, otherParams.strokeMovement);
    cell(report, sheet_1, 143, 13, valveInfo.materialStuffingBoxSeal);


    // Лист 3; Страница: Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    cell(report, sheet_1, 148, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, sheet_1, 150, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, sheet_1, 152, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));
    cell(report, sheet_1, 154, 8, telemetryStore.cyclicTestRecord.sequenceShutoff);
    cell(report, sheet_1, 156, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecShutoff)
                                               .toString("mm:ss.zzz"));

    // Лист 3; Страница: Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    cell(report, sheet_1, 164, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));
    cell(report, sheet_1, 166, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));

    const auto& ons = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto& offs = telemetryStore.cyclicTestRecord.doOffCounts;

    const quint16 baseRow = 164;
    const quint16 rowStep = 2;

    // Выводим все DO, даже если 0 — так шаблон всегда совпадает (DOi -> строка baseRow + i*rowStep)
    for (int i = 0; i < ons.size(); ++i) {
        const quint16 row = baseRow + quint16(i) * rowStep;

        cell(report, sheet_1, row, 10, QString::number(ons.value(i, 0)));
        cell(report, sheet_1, row, 13, QString::number(offs.value(i, 0)));
    }

    // Лист 3; Страница: Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    cell(report,
        "Отчет ЦТ", 172, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        "Отчет ЦТ", 172, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    );
    cell(report,
        "Отчет ЦТ", 172, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    );
    cell(report,
        "Отчет ЦТ", 174, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        "Отчет ЦТ", 174, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    );
    cell(report,
        "Отчет ЦТ", 174, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    );

    // Лист Отчет ЦТ; Страница: 3; Блок: Исполнитель
    cell(report, sheet_1, 181, 4, objectInfo.FIO);
    // Лист Отчет ЦТ; Страница: 3; Блок: Дата
    cell(report, sheet_1, 185, 12, otherParams.date);

    // Лист: Результат теста шаговой реакции; Страница: 1; Блок: Данные по объекту
    cell(report, sheet_2, 4, 4, objectInfo.object);
    cell(report, sheet_2, 5, 4, objectInfo.manufactory);
    cell(report, sheet_2, 6, 4, objectInfo.department);

    // Лист: Результат теста шаговой реакции; Страница: 1; Блок: Краткая спецификация на клапан
    cell(report, sheet_2, 4, 13, valveInfo.positionNumber);
    cell(report, sheet_2, 5, 13, valveInfo.serialNumber);
    cell(report, sheet_2, 6, 13, valveInfo.valveModel);
    cell(report, sheet_2, 7, 13, valveInfo.manufacturer);
    cell(report, sheet_2, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_2, 9, 13, valveInfo.positionerModel);
    cell(report, sheet_2, 10, 13, valveInfo.solenoidValveModel);
    cell(report, sheet_2, 11, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, sheet_2, 12, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_2, 13, 13, otherParams.safePosition);
    cell(report, sheet_2, 14, 13, valveInfo.driveModel);
    cell(report, sheet_2, 15, 13, otherParams.strokeMovement);
    cell(report, sheet_2, 16, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    image(report, sheet_2, 20, 2, imageChartStep); // график зависимости ход штока/управляющий сигнал мА

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    quint16 row = 57;
    for (auto &sr : telemetryStore.stepResults) {
        cell(report,
            sheet_2, row, 3, QString("%1–%2").arg(sr.from).arg(sr.to)
        );
        cell(report,
            sheet_2, row, 4, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz")
        );
        cell(report,
            sheet_2, row, 5,
            QString("%1").arg(sr.overshoot, 0, 'f', 2)
        );
        ++row;
    }

    cell(report, sheet_2, 78, 12, otherParams.date);

    // Страница: Отчет; Блок: Данные по объекту
    cell(report, sheet_3, 4, 4, objectInfo.object);
    cell(report, sheet_3, 5, 4, objectInfo.manufactory);
    cell(report, sheet_3, 6, 4, objectInfo.department);

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    cell(report, sheet_3, 4, 13, valveInfo.positionNumber);
    cell(report, sheet_3, 5, 13, valveInfo.serialNumber);
    cell(report, sheet_3, 6, 13, valveInfo.valveModel);
    cell(report, sheet_3, 7, 13, valveInfo.manufacturer);
    cell(report, sheet_3, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_3, 9, 13, valveInfo.positionerModel);
    cell(report, sheet_3, 10, 13, valveInfo.solenoidValveModel);
    cell(report, sheet_3, 11, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, sheet_3,12, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_3, 13, 13, otherParams.safePosition);
    cell(report, sheet_3, 14, 13, valveInfo.driveModel);
    cell(report, sheet_3, 15, 13, otherParams.strokeMovement);
    cell(report, sheet_3, 16, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Отчет; Блок: Результат испытаний
    cell(report, sheet_3, 22, 5,
                           QString("%1")
                                .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2));

    cell(report, sheet_3, 22, 8,
                           QString("%1")
                                .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2));
    cell(report, sheet_3, 22, 11, resultOk(telemetryStore.crossingStatus.dynamicError));

    // cell(report, sheet_3, 24, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // cell(report, sheet_3, 24, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    cell(report, sheet_3, 26, 5,
                           QString("%1")
                                .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2));
    cell(report, sheet_3, 26, 8, valveInfo.strokValve);
    cell(report, sheet_3, 26, 11, resultOk(telemetryStore.crossingStatus.range));

    cell(report,
        sheet_3, 28, 5,
        QString("%1—%2")
            .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
    );
    cell(report, sheet_3, 28, 8, QString("%1–%2").arg(valveInfo.driveRangeLow, valveInfo.driveRangeHigh));
    cell(report, sheet_3, 28, 11, resultOk(telemetryStore.crossingStatus.spring));

    cell(report, sheet_3, 30, 5,
        QString("%1—%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    );

    cell(report,  sheet_3, 32, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    );

    cell(report,  sheet_3, 34, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    );
    cell(report, sheet_3, 30, 11, resultLimit(telemetryStore.crossingStatus.frictionPercent));


    cell(report,  sheet_3, 48, 5,telemetryStore.strokeTestRecord.timeForwardMs
    );

    cell(report,
        sheet_3, 48, 8,telemetryStore.strokeTestRecord.timeBackwardMs
    );

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, sheet_3, 62, 12, otherParams.date);
    // Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, sheet_3, 70, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики клапана, поз.
    image(report, sheet_3, 82, 1, imageChartTask); // график зависимости ход штока/управляющий сигнал мА
    image(report, sheet_3, 106, 1, imageChartPressure); // график зависимости ход штока/давление в приводе
    image(report, sheet_3, 132, 1, imageChartFriction); // график трения

    // Страница: Отчет; Блок: Дата
    cell(report, sheet_3, 145, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
