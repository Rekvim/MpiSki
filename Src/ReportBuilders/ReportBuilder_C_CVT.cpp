#include "ReportBuilder_C_CVT.h"
#include "ReportWriter.h"
#include "ReportBlocks/ObjectInfoBlock.h"
#include "ReportBlocks/ValveSpecBlock.h"
#include "ReportBlocks/StrokeSummaryBlock.h"
#include "Src/ReportBuilders/ReportBlocks/StepReactionLayout.h"

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
    ReportWriter writer(report);

    ReportContext ctx{
        telemetryStore,
        objectInfo,
        valveInfo,
        otherParams,
        imageChartTask,
        imageChartPressure,
        imageChartFriction,
        imageChartStep
    };

    // Лист: Отчет ЦТ; Страница: 1; Блок: Данные по объекту
    writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

    ObjectInfoBlock({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 4, 13, true, false}).build(writer, ctx);
    StrokeSummaryBlock({m_sheetCyclicTests, 19, 8, 2}).build(writer, ctx);

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

    writer.cell(m_sheetCyclicTests, 56, 4, objectInfo.FIO);
    writer.cell(m_sheetCyclicTests, 60, 12, otherParams.date);

    ObjectInfoBlock({m_sheetCyclicTests, 65, 4}).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 65, 13, true, false}).build(writer, ctx);
    StrokeSummaryBlock({m_sheetCyclicTests, 80, 8, 2}).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 117, 4, objectInfo.FIO);
    writer.cell(m_sheetCyclicTests, 121, 12, otherParams.date);

    //
    // Лист: Результат теста шаговой реакции;
    //

    writer.cell(m_sheetStepReactionTest, 1, 9, valveInfo.positionNumber);

    ObjectInfoBlock({m_sheetStepReactionTest, 4, 4}).build(writer, ctx);
    ValveSpecBlock({m_sheetStepReactionTest, 4, 13, true, false}).build(writer, ctx);
    StepReactionBlock({m_sheetStepReactionTest,
                       18,
                       2,
                       55
                      }).build(writer, ctx);

    writer.cell(m_sheetStepReactionTest, 75, 12, otherParams.date);

    //
    // Страница: Отчет; Блок: Данные по объекту
    //

    writer.cell(m_sheetTechnicalInspection, 1, 9, valveInfo.positionNumber);
    ObjectInfoBlock({m_sheetTechnicalInspection, 4, 4}).build(writer, ctx);
    ValveSpecBlock({m_sheetTechnicalInspection, 4, 13, true, false}).build(writer, ctx);

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
    writer.cell(m_sheetTechnicalInspection, 60, 12, otherParams.date);
    writer.cell(m_sheetTechnicalInspection, 68, 4, objectInfo.FIO);

    // Страница: Отчет; Блок: Диагностические графики
    writer.image(m_sheetTechnicalInspection, 78, 1, imageChartTask);
    writer.image(m_sheetTechnicalInspection, 103, 1, imageChartPressure);
    writer.image(m_sheetTechnicalInspection, 128, 1, imageChartFriction);

    // Страница: Отчет; Блок: Дата
    writer.cell( m_sheetTechnicalInspection, 153, 12, otherParams.date);

    writer.validation("=ЗИП!$A$1:$A$37", "J50:J59");
    writer.validation("=Заключение!$B$1:$B$4", "E36");
    writer.validation("=Заключение!$C$1:$C$3", "E38");
    writer.validation("=Заключение!$E$1:$E$4", "E40");
    writer.validation("=Заключение!$D$1:$D$5", "E42");
    writer.validation("=Заключение!$F$3", "E44");
}
