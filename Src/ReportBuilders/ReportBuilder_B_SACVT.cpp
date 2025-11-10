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
    report.data.push_back({sheet_1, 12, 13, QString("%1 бар")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});
    report.data.push_back({sheet_1, 13, 13, otherParams.safePosition});
    report.data.push_back({sheet_1, 14, 13, valveInfo.driveModel});
    report.data.push_back({sheet_1, 15, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_1, 16, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    report.data.push_back({sheet_1, 21, 8, telemetryStore.strokeTestRecord.timeForwardMs});
    report.data.push_back({sheet_1, 23, 8, telemetryStore.strokeTestRecord.timeBackwardMs});
    report.data.push_back({sheet_1, 25, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 27, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 29, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec)
                                               .toString("mm:ss.zzz")});

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
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                });
                report.data.push_back({
                    sheet_1, row, 11,
                    QString("%1")
                        .arg(a.maxFwdCycle + 1)
                });
            } else {
                // нет данных
                report.data.push_back({ sheet_1, row, 8, QString() });
                report.data.push_back({ sheet_1, row, 11, QString() });
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                report.data.push_back({
                    sheet_1, row, 12,
                    QString("%1")
                        .arg(a.minRevVal,   0, 'f', 2)
                });
                report.data.push_back({
                    sheet_1, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                });
            } else {
                report.data.push_back({ sheet_1, row, 12, QString() });
                report.data.push_back({ sheet_1, row, 15, QString() });
            }
        }
    }

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_1, 58, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_1, 62, 12, otherParams.date});


    // Лист Отчет ЦТ; Страница: 2; Блок: Данные по объекту
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
    report.data.push_back({sheet_1, 76, 13, QString("%1")
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

    // Лист: Отчет ЦТ; Страница: 3; Блок: Исполнитель
    report.data.push_back({sheet_1, 122, 4, objectInfo.FIO});
    // Лист: Отчет ЦТ; Страница: 3; Блок: Дата
    report.data.push_back({sheet_1, 126, 12, otherParams.date});


    // Лист: Отчет ЦТ; Страница: 3; Блок: Циклические испытания соленоидного клапана
    report.data.push_back({sheet_1, 131, 4, objectInfo.object});
    report.data.push_back({sheet_1, 132, 4, objectInfo.manufactory});
    report.data.push_back({sheet_1, 133, 4, objectInfo.department});

    // Лист: Отчет ЦТ; Страница: 3; Блок: Краткая спецификация на клапан
    report.data.push_back({sheet_1, 131, 13, valveInfo.positionNumber});
    report.data.push_back({sheet_1, 132, 13, valveInfo.serialNumber});
    report.data.push_back({sheet_1, 133, 13, valveInfo.valveModel});
    report.data.push_back({sheet_1, 134, 13, valveInfo.manufacturer});
    report.data.push_back({sheet_1, 135, 13, QString("%1 / %2")
                                                .arg(valveInfo.DN)
                                                .arg(valveInfo.PN)});
    report.data.push_back({sheet_1, 136, 13, valveInfo.positionerModel});
    report.data.push_back({sheet_1, 137, 13, valveInfo.solenoidValveModel});
    report.data.push_back({sheet_1, 138, 13, QString("%1 / %2")
                                                .arg(valveInfo.limitSwitchModel)
                                                .arg(valveInfo.positionSensorModel)});
    report.data.push_back({sheet_1, 139, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2)});
    report.data.push_back({sheet_1, 140, 13, otherParams.safePosition});
    report.data.push_back({sheet_1, 141, 13, valveInfo.driveModel});
    report.data.push_back({sheet_1, 142, 13, otherParams.strokeMovement});
    report.data.push_back({sheet_1, 143, 13, valveInfo.materialStuffingBoxSeal});

    //  Лист: Отчет ЦТ; Страница: 3; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    report.data.push_back({sheet_1, 148, 8, telemetryStore.strokeTestRecord.timeForwardMs});
    report.data.push_back({sheet_1, 150, 8, telemetryStore.strokeTestRecord.timeBackwardMs});
    report.data.push_back({sheet_1, 152, 8, QString::number(telemetryStore.cyclicTestRecord.cycles)});
    report.data.push_back({sheet_1, 154, 8, telemetryStore.cyclicTestRecord.sequence});
    report.data.push_back({sheet_1, 156, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec)
                                               .toString("mm:ss.zzz")});

    //  Лист: Отчет ЦТ; Страница: 3; Блок: Циклические испытания соленоидного клапана
    report.data.push_back({
        "Отчет ЦТ", 164, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 166, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });

    const auto &ons  = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto &offs = telemetryStore.cyclicTestRecord.doOffCounts;
    for (int i = 0; i < ons.size(); ++i) {
        if (ons[i] == 0 && offs.value(i, 0) == 0)
            continue;

        quint16 row = 164 + quint16(i) * 2;
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
        "Отчет ЦТ", 172, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 172, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 172, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 174, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 174, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 174, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    });
    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({sheet_1, 181, 4, objectInfo.FIO});
    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({sheet_1, 185, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
