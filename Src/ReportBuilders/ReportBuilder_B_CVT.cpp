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
    cell(report, m_sheetTechnicalInspection, 1, 9, valveInfo.positionNumber);

    // Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 4, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 6, 4, objectInfo.department);

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
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

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    cell(report, m_sheetTechnicalInspection, 19, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetTechnicalInspection, 21, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetTechnicalInspection, 23, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, m_sheetTechnicalInspection, 25, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, m_sheetTechnicalInspection, 27, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory).toString("mm:ss.zzz"));

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
                    a.maxFwdVal = rec.maxForwardValue;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход
                if (rec.maxReverseCycle >= 0) {
                    a.minRevVal = rec.maxReverseValue;
                    a.minRevCycle = rec.maxReverseCycle;
                }
                aggMap.insert(rec.rangePercent, a);

            } else {
                // уже есть — обновляем экстремумы
                Agg &a = it.value();
                // прямой ход — берём глобальный максимум
                if (rec.maxForwardCycle >= 0
                    && rec.maxForwardValue > a.maxFwdVal) {
                    a.maxFwdVal = rec.maxForwardValue;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход — берём глобальный минимум
                if (rec.maxReverseCycle >= 0
                    && rec.maxReverseValue < a.minRevVal) {
                    a.minRevVal = rec.maxReverseValue;
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
                        .arg(a.minRevVal, 0, 'f', 2)
                );
                cell(report,
                    m_sheetTechnicalInspection, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                );
            } else {
                cell(report,  m_sheetTechnicalInspection, row, 12, QString());
                cell(report,  m_sheetTechnicalInspection, row, 15, QString());
            }
        }
    }

    // Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, m_sheetTechnicalInspection, 56, 4, objectInfo.FIO);

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 60, 12, otherParams.date);

    // Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, m_sheetTechnicalInspection, 65, 4, objectInfo.object);
    cell(report, m_sheetTechnicalInspection, 66, 4, objectInfo.manufactory);
    cell(report, m_sheetTechnicalInspection, 67, 4, objectInfo.department);

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, m_sheetTechnicalInspection, 65, 13, valveInfo.positionNumber);
    cell(report, m_sheetTechnicalInspection, 66, 13, valveInfo.serialNumber);
    cell(report, m_sheetTechnicalInspection, 67, 13, valveInfo.valveModel);
    cell(report, m_sheetTechnicalInspection, 68, 13, valveInfo.manufacturer);
    cell(report, m_sheetTechnicalInspection, 69, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetTechnicalInspection, 70, 13, valveInfo.positionerModel);
    cell(report, m_sheetTechnicalInspection, 71, 13, QString("%1")
                                                .arg(telemetryStore.supplyRecord.pressure_bar, 0, 'f', 2));
    cell(report, m_sheetTechnicalInspection, 72, 13, otherParams.safePosition);
    cell(report, m_sheetTechnicalInspection, 73, 13, valveInfo.driveModel);
    cell(report, m_sheetTechnicalInspection, 74, 13, otherParams.strokeMovement);
    cell(report, m_sheetTechnicalInspection, 75, 13, valveInfo.materialStuffingBoxSeal);

    // Страница:Отчет ЦТ; Блок: Результат испытаний позиционера
    cell(report, m_sheetTechnicalInspection, 80, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetTechnicalInspection, 82, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetTechnicalInspection, 84, 8, telemetryStore.cyclicTestRecord.sequenceRegulatory);
    cell(report, m_sheetTechnicalInspection, 86, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesRegulatory));
    cell(report, m_sheetTechnicalInspection, 88, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecRegulatory).toString("mm:ss.zzz"));

    // Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, m_sheetTechnicalInspection, 117, 4, objectInfo.FIO);
    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetTechnicalInspection, 121, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
