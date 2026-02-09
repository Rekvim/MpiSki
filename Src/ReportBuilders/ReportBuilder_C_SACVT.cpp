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

    cell(report, m_sheetTechnicalInspection, 1, 9, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 1, 9, valveInfo.positionNumber);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 4, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 6, 4, objectInfo.department);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Краткая спецификация на клапан
    cell(report, m_sheetTechnicalInspection, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetTechnicalInspection, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetTechnicalInspection, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetTechnicalInspection, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetTechnicalInspection, 9, 13, valveInfo.positionerModel);
    cell(report, m_sheetTechnicalInspection, 10, 13, valveInfo.solenoidValveModel);
    cell(report, m_sheetTechnicalInspection, 11, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, m_sheetTechnicalInspection, 12, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 13, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 14, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 15, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 16, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Результат испытаний позиционера
    cell(report, m_sheetTechnicalInspection, 21, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetTechnicalInspection, 23, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetTechnicalInspection, 25, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, m_sheetTechnicalInspection, 27, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, m_sheetTechnicalInspection, 29, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory).toString("mm:ss.zzz"));


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
                m_sheetTechnicalInspection, row, 2,
                QString::number(a.rangePercent)
            );

            // Прямой ход (максимум)
            if (a.maxFwdCycle >= 0) {
                cell(report,
                    m_sheetTechnicalInspection, row, 8,
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                );
                cell(report,
                    m_sheetTechnicalInspection, row, 11,
                    QString("%1")
                        .arg(a.maxFwdCycle + 1)
                );
            } else {
                // нет данных
                cell(report, m_sheetTechnicalInspection, row, 8, QString());
                cell(report, m_sheetTechnicalInspection, row, 11, QString());
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                cell(report,
                    m_sheetTechnicalInspection, row, 12,
                    QString("%1")
                        .arg(a.minRevVal,   0, 'f', 2)
                );
                cell(report,
                    m_sheetTechnicalInspection, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                );
            } else {
                cell(report, m_sheetTechnicalInspection, row, 12, QString());
                cell(report, m_sheetTechnicalInspection, row, 15, QString());
            }
        }
    }

    // Лист 2; Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 68, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 69, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 70, 4, objectInfo.department);

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, m_sheetTechnicalInspection, 68, 13, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 69, 13, valveInfo.serialNumber);
    cell(report, m_sheetTechnicalInspection, 70, 13, valveInfo.valveModel);
    cell(report, m_sheetTechnicalInspection, 71, 13, valveInfo.manufacturer);
    cell(report, m_sheetTechnicalInspection, 72, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetTechnicalInspection, 73, 13, valveInfo.positionerModel);
    cell(report, m_sheetTechnicalInspection, 74, 13, valveInfo.solenoidValveModel);
    cell(report, m_sheetTechnicalInspection, 75, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, m_sheetTechnicalInspection, 76, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 77, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 78, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 79, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 80, 13, valveInfo.materialStuffingBoxSeal);

    // Лист: Отчет ЦТ; Страница: 1; Блок: Результат испытаний позиционера
    cell(report, m_sheetTechnicalInspection, 85, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetTechnicalInspection, 87, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetTechnicalInspection, 89, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, m_sheetTechnicalInspection, 91, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, m_sheetTechnicalInspection, 93, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory).toString("mm:ss.zzz"));


    // Лист 2; Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, m_sheetTechnicalInspection, 122, 4, objectInfo.FIO);
    // Лист 2; Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 126, 12, otherParams.date);


    // Лист 3; Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 131, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 132, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 133, 4, objectInfo.department);

    // Лист 3; Страница: Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, m_sheetTechnicalInspection, 131, 13, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 132, 13, valveInfo.serialNumber);
    cell(report, m_sheetTechnicalInspection, 133, 13, valveInfo.valveModel);
    cell(report, m_sheetTechnicalInspection, 134, 13, valveInfo.manufacturer);
    cell(report, m_sheetTechnicalInspection, 135, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetTechnicalInspection, 136, 13, valveInfo.positionerModel);
    cell(report, m_sheetTechnicalInspection, 137, 13, valveInfo.solenoidValveModel);
    cell(report, m_sheetTechnicalInspection, 138, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, m_sheetTechnicalInspection, 139, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 140, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 141, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 142, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 143, 13, valveInfo.materialStuffingBoxSeal);


    // Лист 3; Страница: Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    cell(report, m_sheetTechnicalInspection, 148, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetTechnicalInspection, 150, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetTechnicalInspection, 152, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));
    cell(report, m_sheetTechnicalInspection, 154, 8, telemetryStore.cyclicTestRecord.sequenceShutoff);
    cell(report, m_sheetTechnicalInspection, 156, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecShutoff)
                                               .toString("mm:ss.zzz"));

    // Лист 3; Страница: Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    cell(report, m_sheetTechnicalInspection, 164, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));
    cell(report, m_sheetTechnicalInspection, 166, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));

    const auto& ons = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto& offs = telemetryStore.cyclicTestRecord.doOffCounts;

    const quint16 baseRow = 164;
    const quint16 rowStep = 2;

    // Выводим все DO, даже если 0 — так шаблон всегда совпадает (DOi -> строка baseRow + i*rowStep)
    for (int i = 0; i < ons.size(); ++i) {
        const quint16 row = baseRow + quint16(i) * rowStep;

        cell(report, m_sheetTechnicalInspection, row, 10, QString::number(ons.value(i, 0)));
        cell(report, m_sheetTechnicalInspection, row, 13, QString::number(offs.value(i, 0)));
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
    cell(report, m_sheetTechnicalInspection, 181, 4, objectInfo.FIO);
    // Лист Отчет ЦТ; Страница: 3; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 185, 12, otherParams.date);

    // Лист: Результат теста шаговой реакции; Страница: 1; Блок: Данные по объекту
    cell(report, m_sheetStepReactionTest, 1, 9, valveInfo.positionNumber);
    cell(report, m_sheetStepReactionTest, 4, 4, objectInfo.object);
    cell(report, m_sheetStepReactionTest, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetStepReactionTest, 6, 4, objectInfo.department);

    // Лист: Результат теста шаговой реакции; Страница: 1; Блок: Краткая спецификация на клапан
    cell(report, m_sheetStepReactionTest, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetStepReactionTest, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetStepReactionTest, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetStepReactionTest, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetStepReactionTest, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetStepReactionTest, 9, 13, valveInfo.positionerModel);
    cell(report, m_sheetStepReactionTest, 10, 13, valveInfo.solenoidValveModel);
    cell(report, m_sheetStepReactionTest, 11, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, m_sheetStepReactionTest, 12, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetStepReactionTest, 13, 13, otherParams.safePosition);
    cell(report, m_sheetStepReactionTest, 14, 13, valveInfo.driveModel);
    cell(report, m_sheetStepReactionTest, 15, 13, otherParams.strokeMovement);
    cell(report, m_sheetStepReactionTest, 16, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Результат теста шаговой реакции; Блок: График теста шаговой реакции
    image(report, m_sheetStepReactionTest, 20, 2, imageChartStep); // график зависимости ход штока/управляющий сигнал мА

    // Страница: Результат теста шаговой реакции; Блок: Результат теста шаговой реакции
    quint16 row = 57;
    for (auto &sr : telemetryStore.stepResults) {
        cell(report,
            m_sheetStepReactionTest, row, 3, QString("%1–%2").arg(sr.from).arg(sr.to)
        );
        cell(report,
            m_sheetStepReactionTest, row, 5, QTime(0,0).addMSecs(sr.T_value).toString("m:ss.zzz")
        );
        cell(report,
            m_sheetStepReactionTest, row, 7,
            QString("%1").arg(sr.overshoot, 0, 'f', 2)
        );
        ++row;
    }

    cell(report, m_sheetStepReactionTest, 75, 12, otherParams.date);

    // Страница: Отчет; Блок: Данные по объекту
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
    cell(report, m_sheetTechnicalInspection, 10, 13, valveInfo.solenoidValveModel);
    cell(report, m_sheetTechnicalInspection, 11, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, m_sheetTechnicalInspection,12, 13, QString("%1").arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 13, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 14, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 15, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 16, 13, valveInfo.materialStuffingBoxSeal);

    // Страница: Отчет; Блок: Результат испытаний
    cell(report, m_sheetTechnicalInspection, 22, 5,
                           QString("%1")
                                .arg(telemetryStore.mainTestRecord.dynamicErrorReal, 0, 'f', 2));

    cell(report, m_sheetTechnicalInspection, 22, 8,
                           QString("%1")
                                .arg(valveInfo.dinamicErrorRecomend, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 22, 11, resultOk(telemetryStore.crossingStatus.dynamicError));

    // cell(report, m_sheetTechnicalInspection, 24, 5, telemetryStore.dinamicRecord.dinamicIpReal});
    // cell(report, m_sheetTechnicalInspection, 24, 8, telemetryStore.dinamicRecord.dinamicIpRecomend});

    cell(report, m_sheetTechnicalInspection, 26, 5,
                           QString("%1")
                                .arg(telemetryStore.valveStrokeRecord.real, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 26, 8, valveInfo.strokValve);
    cell(report, m_sheetTechnicalInspection, 26, 11, resultOk(telemetryStore.crossingStatus.range));

    cell(report,
        m_sheetTechnicalInspection, 28, 5,
        QString("%1—%2")
            .arg(telemetryStore.mainTestRecord.springLow, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.springHigh, 0, 'f', 2)
    );
    cell(report, m_sheetTechnicalInspection, 28, 8,
        QString("%1–%2")
            .arg(valveInfo.driveRangeLow, 0, 'f', 2)
            .arg(valveInfo.driveRangeHigh, 0, 'f', 2)
    );
    cell(report, m_sheetTechnicalInspection, 28, 11, resultOk(telemetryStore.crossingStatus.spring));

    cell(report, m_sheetTechnicalInspection, 30, 5,
        QString("%1—%2")
            .arg(telemetryStore.mainTestRecord.lowLimitPressure, 0, 'f', 2)
            .arg(telemetryStore.mainTestRecord.highLimitPressure, 0, 'f', 2)
    );

    cell(report,  m_sheetTechnicalInspection, 32, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionPercent, 0, 'f', 2)
    );

    cell(report,  m_sheetTechnicalInspection, 34, 5,
        QString("%1")
            .arg(telemetryStore.mainTestRecord.frictionForce, 0, 'f', 3)
    );
    cell(report, m_sheetTechnicalInspection, 30, 11, resultLimit(telemetryStore.crossingStatus.frictionPercent));


    cell(report,  m_sheetTechnicalInspection, 48, 5,telemetryStore.strokeTestRecord.timeForwardMs
    );

    cell(report,
        m_sheetTechnicalInspection, 48, 8,telemetryStore.strokeTestRecord.timeBackwardMs
    );

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 62, 12, otherParams.date);
    // Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, m_sheetTechnicalInspection, 70, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики клапана, поз.
    image(report, m_sheetTechnicalInspection, 82, 1, imageChartTask); // график зависимости ход штока/управляющий сигнал мА
    image(report, m_sheetTechnicalInspection, 106, 1, imageChartPressure); // график зависимости ход штока/давление в приводе
    image(report, m_sheetTechnicalInspection, 132, 1, imageChartFriction); // график трения

    // Страница: Отчет; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 145, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
