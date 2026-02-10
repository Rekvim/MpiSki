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
    // Лист: Отчет ЦТ; Страница: 1; Блок: Данные по объекту
    cell(report, m_sheetCyclicTests, 1, 9, valveInfo.positionNumber);
    cell(report, m_sheetCyclicTests, 4, 4, objectInfo.object);
    cell(report, m_sheetCyclicTests, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetCyclicTests, 6, 4, objectInfo.department);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Краткая спецификация на клапан
    cell(report, m_sheetCyclicTests, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetCyclicTests, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetCyclicTests, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetCyclicTests, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetCyclicTests, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetCyclicTests, 9, 13, valveInfo.positionerModel);
    cell(report, m_sheetCyclicTests, 10, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetCyclicTests, 11, 13, otherParams.safePosition);
    cell(report, m_sheetCyclicTests, 12, 13, valveInfo.driveModel);
    cell(report, m_sheetCyclicTests, 13, 13, otherParams.strokeMovement);
    cell(report, m_sheetCyclicTests, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Результат испытаний позиционера
    cell(report, m_sheetCyclicTests, 19, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetCyclicTests, 21, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetCyclicTests, 23, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, m_sheetCyclicTests, 25, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, m_sheetCyclicTests, 27, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory)
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
                m_sheetCyclicTests, row, 2,
                QString::number(a.rangePercent)
            );

            // Прямой ход (максимум)
            if (a.maxFwdCycle >= 0) {
                cell(report,m_sheetCyclicTests, row, 8,
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                );
                cell(report, m_sheetCyclicTests, row, 11,
                    QString("%1")
                        .arg(a.maxFwdCycle + 1)
                );
            } else {
                // нет данных
                cell(report, m_sheetCyclicTests, row, 8, QString());
                cell(report, m_sheetCyclicTests, row, 11, QString());
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                cell(report, m_sheetCyclicTests, row, 12,
                    QString("%1")
                        .arg(a.minRevVal,   0, 'f', 2)
                );
                cell(report,
                    m_sheetCyclicTests, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                );
            } else {
                cell(report, m_sheetCyclicTests, row, 12, QString());
                cell(report, m_sheetCyclicTests, row, 15, QString());
            }
        }
    }

    // Лист: Отчет ЦТ; Страница: 1; Блок: Исполнитель
    cell(report, m_sheetCyclicTests, 56, 4, objectInfo.FIO);
    cell(report, m_sheetCyclicTests, 60, 12, otherParams.date);

    // Лист: Отчет ЦТ; Страница: 2; Блок: Данные по объекту
    cell(report, m_sheetCyclicTests, 65, 4, objectInfo.object);
    cell(report, m_sheetCyclicTests, 66, 4, objectInfo.manufactory);
    cell(report, m_sheetCyclicTests, 67, 4, objectInfo.department);

    // Лист: Отчет ЦТ; Страница: 2; Блок: Краткая спецификация на клапан
    cell(report, m_sheetCyclicTests, 65, 13, valveInfo.positionNumber);
    cell(report, m_sheetCyclicTests, 66, 13, valveInfo.serialNumber);
    cell(report, m_sheetCyclicTests, 67, 13, valveInfo.valveModel);
    cell(report, m_sheetCyclicTests, 68, 13, valveInfo.manufacturer);
    cell(report, m_sheetCyclicTests, 69, 13, QString("%1 / %2")
                                               .arg(valveInfo.DN)
                                               .arg(valveInfo.PN));
    cell(report, m_sheetCyclicTests, 70, 13, valveInfo.positionerModel);
    cell(report, m_sheetCyclicTests, 71, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetCyclicTests, 72, 13, otherParams.safePosition);
    cell(report, m_sheetCyclicTests, 73, 13, valveInfo.driveModel);
    cell(report, m_sheetCyclicTests, 74, 13, otherParams.strokeMovement);
    cell(report, m_sheetCyclicTests, 75, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 2; Блок: Результат испытаний позиционера
    cell(report, m_sheetCyclicTests, 80, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetCyclicTests, 82, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetCyclicTests, 84, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, m_sheetCyclicTests, 86, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, m_sheetCyclicTests, 88, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory)
                                               .toString("mm:ss.zzz"));


    // Лист: Отчет ЦТ; Страница: 2; Блок: Исполнитель
    cell(report, m_sheetCyclicTests, 117, 4, objectInfo.FIO);
    // Лист: Отчет ЦТ; Страница: 2; Блок: Дата
    cell(report, m_sheetCyclicTests, 121, 12, otherParams.date);

    // Лист: Результат теста шаговой реакции; Страница: 2; Блок: Данные по объекту
    cell(report, m_sheetStepReactionTest, 1, 9, valveInfo.positionNumber);
    cell(report, m_sheetStepReactionTest, 4, 4, objectInfo.object);
    cell(report, m_sheetStepReactionTest, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetStepReactionTest, 6, 4, objectInfo.department);

    // Страница:Результат теста шаговой реакции; Блок: Краткая спецификация на клапан
    cell(report, m_sheetStepReactionTest, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetStepReactionTest, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetStepReactionTest, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetStepReactionTest, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetStepReactionTest, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetStepReactionTest, 9, 13, valveInfo.positionerModel);
    cell(report, m_sheetStepReactionTest, 10, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetStepReactionTest, 11, 13, otherParams.safePosition);
    cell(report, m_sheetStepReactionTest, 12, 13, valveInfo.driveModel);
    cell(report, m_sheetStepReactionTest, 13, 13, otherParams.strokeMovement);
    cell(report, m_sheetStepReactionTest, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    report.images.push_back({m_sheetStepReactionTest, 18, 2, imageChartStep});

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    {
        quint16 row = 55;
        for (auto &sr : telemetryStore.stepResults) {
            cell(report, m_sheetStepReactionTest, row, 3, QString("%1->%2").arg(sr.from).arg(sr.to));
            cell(report, m_sheetStepReactionTest, row, 5, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz"));
            cell(report, m_sheetStepReactionTest, row, 7, QString("%1").arg(sr.overshoot, 0, 'f', 2));
            ++row;
        }
    }

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetStepReactionTest, 75, 12, otherParams.date);

    // Страница: Отчет; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 1, 9, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 4, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 6, 4, objectInfo.department);

    // Страница:Отчет; Блок: Краткая спецификация на клапан
    cell(report, m_sheetTechnicalInspection, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetTechnicalInspection, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetTechnicalInspection, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetTechnicalInspection, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetTechnicalInspection, 9, 13, valveInfo.positionerModel);
    cell(report, m_sheetTechnicalInspection, 10, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 11, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 12, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 13, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 14, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Отчет; Блок: Результат испытаний
    cell(report, m_sheetTechnicalInspection, 20, 5,
                            QString("%1")
                               .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2));

    cell(report, m_sheetTechnicalInspection, 20, 8,
                           QString("%1")
                                .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 20, 11, resultOk(telemetryStore.crossingStatus.dynamicError));

    // cell(report, m_sheetTechnicalInspection, 22, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // cell(report, m_sheetTechnicalInspection, 22, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    cell(report, m_sheetTechnicalInspection, 24, 5, QString("%1")
                                               .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 24, 8, valveInfo.strokValve);
    cell(report, m_sheetTechnicalInspection, 24, 11, resultOk(telemetryStore.crossingStatus.range));

    cell(report, m_sheetTechnicalInspection, 26, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
    );
    cell(report, m_sheetTechnicalInspection, 26, 8,
        QString("%1–%2")
            .arg(valveInfo.driveRangeLow, 0, 'f', 2)
            .arg(valveInfo.driveRangeHigh, 0, 'f', 2)
    );
    cell(report, m_sheetTechnicalInspection, 26, 11, resultOk(telemetryStore.crossingStatus.spring));

    cell(report, m_sheetTechnicalInspection, 28, 5,
        QString("%1–%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    );

    cell(report,
        m_sheetTechnicalInspection, 30, 5,
        QString("%1")
           .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    );
    cell(report, m_sheetTechnicalInspection, 30, 11, resultLimit(telemetryStore.crossingStatus.frictionPercent));

    cell(report,
        m_sheetTechnicalInspection, 32, 5,
        QString("%1")
           .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    );
    cell(report,
        m_sheetTechnicalInspection, 46, 5, telemetryStore.strokeTestRecord.timeForwardMs
    );
    cell(report,
        m_sheetTechnicalInspection, 46, 8, telemetryStore.strokeTestRecord.timeBackwardMs
    );

    // Дата и Исполнитель
    cell(report, m_sheetTechnicalInspection, 60, 12, otherParams.date);
    cell(report, m_sheetTechnicalInspection, 68, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики
    image(report, m_sheetTechnicalInspection, 78, 1, imageChartTask);
    image(report, m_sheetTechnicalInspection, 103, 1, imageChartPressure);
    image(report, m_sheetTechnicalInspection, 128, 1, imageChartFriction);

    // Страница: Отчет; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 153, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E36"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E38"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E40"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E42"});
    report.validation.push_back({"=Заключение!$F$3", "E44"});
}
