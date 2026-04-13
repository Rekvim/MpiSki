#include "ReportBuilder_B_CVT.h"
#include "Src/ReportBuilders/ReportWriter.h"
#include "Src/ReportBuilders/ReportBlocks/ObjectInfoBlock.h"
#include "Src/ReportBuilders/ReportBlocks/ValveSpecBlock.h"
#include "Src/ReportBuilders/ReportBlocks/CyclicSummaryBlock.h"
#include "Src/ReportBuilders/ReportBlocks/CyclicRangesBlock.h"


ReportBuilder_B_CVT::ReportBuilder_B_CVT() {}

void ReportBuilder_B_CVT::buildReport(
    ReportSaver::Report& report,
    const Telemetry& telemetryStore,
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

    writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

    ObjectInfoBlock({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 4, 13, true, false}).build(writer, ctx);
    CyclicSummaryBlock({m_sheetCyclicTests, 19, 8, 2}, CyclicMode::Regulatory).build(writer, ctx);

    {
        const auto& ranges = telemetryStore.cyclicTestRecord.regulatoryResult.ranges;

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
                    a.maxFwdVal = rec.maxForwardPosition;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход
                if (rec.minReverseCycle >= 0) {
                    a.minRevVal = rec.minReverseCycle;
                    a.minRevCycle = rec.minReverseCycle;
                }
                aggMap.insert(rec.rangePercent, a);

            } else {
                // уже есть — обновляем экстремумы
                Agg &a = it.value();
                // прямой ход — берём глобальный максимум
                if (rec.maxForwardCycle >= 0
                    && rec.maxForwardPosition > a.maxFwdVal) {
                    a.maxFwdVal   = rec.maxForwardPosition;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход — берём глобальный минимум
                if (rec.minReverseCycle >= 0
                    && rec.minReverseCycle < a.minRevVal) {
                    a.minRevVal = rec.minReverseCycle;
                    a.minRevCycle = rec.minReverseCycle;
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
            writer.cell(
                m_sheetCyclicTests, row, 2,
                QString::number(a.rangePercent)
            );

            // Прямой ход (максимум)
            if (a.maxFwdCycle >= 0) {
                writer.cell(
                    m_sheetCyclicTests, row, 8,
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                );
                writer.cell(
                    m_sheetCyclicTests, row, 11,
                    QString("%1")
                        .arg(a.maxFwdCycle + 1)
                );
            } else {
                // нет данных
                writer.cell(m_sheetCyclicTests, row, 8, QString());
                writer.cell(m_sheetCyclicTests, row, 11, QString());
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                writer.cell(
                    m_sheetCyclicTests, row, 12,
                    QString("%1")
                        .arg(a.minRevVal, 0, 'f', 2)
                );
                writer.cell(
                    m_sheetCyclicTests, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                );
            } else {
                writer.cell(m_sheetCyclicTests, row, 12, QString());
                writer.cell(m_sheetCyclicTests, row, 15, QString());
            }
        }
    }

    // CyclicRangesBlock({m_sheetCyclicTests, 33, 2}).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 56, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 60, 12, ctx.params.date);

    // Страница 2
    ObjectInfoBlock({m_sheetCyclicTests, 66, 4}).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 66, 13, false, false}).build(writer, ctx);
    CyclicSummaryBlock({m_sheetCyclicTests, 81, 8, 2 }, CyclicMode::Regulatory).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 118, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 122, 12, ctx.params.date);
}
