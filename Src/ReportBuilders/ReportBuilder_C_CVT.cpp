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
    cell(report, sheet_1, 10, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_1, 11, 13, otherParams.safePosition);
    cell(report, sheet_1, 12, 13, valveInfo.driveModel);
    cell(report, sheet_1, 13, 13, otherParams.strokeMovement);
    cell(report, sheet_1, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Результат испытаний позиционера
    cell(report, sheet_1, 19, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, sheet_1, 21, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, sheet_1, 23, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, sheet_1, 25, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, sheet_1, 27, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory)
                                                     .toString("mm:ss.zzz"));

    // Лист: Отчет ЦТ; Страница: 1; Блок: Циклические испытания позиционера
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

        constexpr quint16 rowStart = 33, rowStep = 2;
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
                cell(report,sheet_1, row, 8,
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                );
                cell(report, sheet_1, row, 11,
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
                cell(report, sheet_1, row, 12,
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

    // Лист: Отчет ЦТ; Страница: 1; Блок: Исполнитель
    cell(report, sheet_1, 56, 4, objectInfo.FIO);
    cell(report, sheet_1, 60, 12, otherParams.date);

    // Лист: Отчет ЦТ; Страница: 2; Блок: Данные по объекту
    cell(report, sheet_1, 65, 4, objectInfo.object);
    cell(report, sheet_1, 66, 4, objectInfo.manufactory);
    cell(report, sheet_1, 67, 4, objectInfo.department);

    // Лист: Отчет ЦТ; Страница: 2; Блок: Краткая спецификация на клапан
    cell(report, sheet_1, 65, 13, valveInfo.positionNumber);
    cell(report, sheet_1, 66, 13, valveInfo.serialNumber);
    cell(report, sheet_1, 67, 13, valveInfo.valveModel);
    cell(report, sheet_1, 68, 13, valveInfo.manufacturer);
    cell(report, sheet_1, 69, 13, QString("%1 / %2")
                                               .arg(valveInfo.DN)
                                               .arg(valveInfo.PN));
    cell(report, sheet_1, 70, 13, valveInfo.positionerModel);
    cell(report, sheet_1, 71, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_1, 72, 13, otherParams.safePosition);
    cell(report, sheet_1, 73, 13, valveInfo.driveModel);
    cell(report, sheet_1, 74, 13, otherParams.strokeMovement);
    cell(report, sheet_1, 75, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 2; Блок: Результат испытаний позиционера
    cell(report, sheet_1, 80, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, sheet_1, 82, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, sheet_1, 84, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, sheet_1, 86, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, sheet_1, 88, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory)
                                               .toString("mm:ss.zzz"));


    // Лист: Отчет ЦТ; Страница: 2; Блок: Исполнитель
    cell(report, sheet_1, 117, 4, objectInfo.FIO);
    // Лист: Отчет ЦТ; Страница: 2; Блок: Дата
    cell(report, sheet_1, 121, 12, otherParams.date);

    // Лист: Результат теста шаговой реакции; Страница: 2; Блок: Данные по объекту
    cell(report, sheet_2, 4, 4, objectInfo.object);
    cell(report, sheet_2, 5, 4, objectInfo.manufactory);
    cell(report, sheet_2, 6, 4, objectInfo.department);

    // Страница:Результат теста шаговой реакции; Блок: Краткая спецификация на клапан
    cell(report, sheet_2, 4, 13, valveInfo.positionNumber);
    cell(report, sheet_2, 5, 13, valveInfo.serialNumber);
    cell(report, sheet_2, 6, 13, valveInfo.valveModel);
    cell(report, sheet_2, 7, 13, valveInfo.manufacturer);
    cell(report, sheet_2, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, sheet_2, 9, 13, valveInfo.positionerModel);
    cell(report, sheet_2, 10, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_2, 11, 13, otherParams.safePosition);
    cell(report, sheet_2, 12, 13, valveInfo.driveModel);
    cell(report, sheet_2, 13, 13, otherParams.strokeMovement);
    cell(report, sheet_2, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({sheet_2, 18, 2, imageChartStep});

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    {
        quint16 row = 55;
        for (auto &sr : telemetryStore.stepResults) {
            cell(report, sheet_2, row, 3, QString("%1->%2").arg(sr.from).arg(sr.to));
            cell(report, sheet_2, row, 4, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz"));
            cell(report, sheet_2, row, 5, QString("%1").arg(sr.overshoot, 0, 'f', 2));
            ++row;
        }
    }

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, sheet_2, 76, 12, otherParams.date);

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
    cell(report, sheet_3, 10, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, sheet_3, 11, 13, otherParams.safePosition);
    cell(report, sheet_3, 12, 13, valveInfo.driveModel);
    cell(report, sheet_3, 13, 13, otherParams.strokeMovement);
    cell(report, sheet_3, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Отчет; Блок: Результат испытаний
    cell(report, sheet_3, 20, 5,
                            QString("%1")
                               .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2));

    cell(report, sheet_3, 20, 8,
                           QString("%1")
                                .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2));
    cell(report, sheet_3, 20, 11, resultOk(telemetryStore.crossingStatus.dynamicError));

    // cell(report, sheet_3, 22, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // cell(report, sheet_3, 22, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    cell(report, sheet_3, 24, 5, QString("%1")
                                               .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2));
    cell(report, sheet_3, 24, 8, valveInfo.strokValve);
    cell(report, sheet_3, 24, 11, resultOk(telemetryStore.crossingStatus.range));

    cell(report, sheet_3, 26, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
    );
    cell(report, sheet_3, 26, 11, resultOk(telemetryStore.crossingStatus.spring));

    cell(report, sheet_3, 26, 8, valveInfo.driveRecomendRange);

    cell(report, sheet_3, 28, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    );

    cell(report,
        sheet_3, 30, 5, 
        QString("%1")
           .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    );
    cell(report, sheet_3, 30, 11, resultLimit(telemetryStore.crossingStatus.frictionPercent));

    cell(report,
        sheet_3, 32, 5,
        QString("%1")
           .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    );
    cell(report,
        sheet_3, 46, 5, telemetryStore.strokeTestRecord.timeForwardMs
    );
    cell(report,
        sheet_3, 46, 8, telemetryStore.strokeTestRecord.timeBackwardMs
    );

    // Дата и Исполнитель
    cell(report, sheet_3, 60, 12, otherParams.date);
    cell(report, sheet_3, 68, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики
    image(report, sheet_3,  78, 1, imageChartTask);
    image(report, sheet_3, 103, 1, imageChartPressure);
    image(report, sheet_3, 128, 1, imageChartFriction);

    // Страница: Отчет; Блок: Дата
    cell(report, sheet_3, 153, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
