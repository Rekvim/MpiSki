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

    const auto& ranges = telemetryStore.cyclicTestRecord.regulatoryResult.ranges;

    struct Agg {
        qreal rangePercent;

        qreal maxFwdVal = std::numeric_limits<qreal>::lowest();
        int   maxFwdCycle = -1;

        qreal minRevVal = std::numeric_limits<qreal>::max();
        int   minRevCycle = -1;
    };

    QMap<qreal, Agg> aggMap;

    // --- агрегация ---
    for (const auto& r : ranges)
    {
        auto it = aggMap.find(r.rangePercent);

        if (it == aggMap.end())
        {
            Agg a;
            a.rangePercent = r.rangePercent;

            if (r.maxForwardCycle >= 0) {
                a.maxFwdVal = r.maxForwardPosition;
                a.maxFwdCycle = r.maxForwardCycle;
            }

            if (r.minBackwardCycle >= 0) {
                a.minRevVal = r.minBackwardPosition;
                a.minRevCycle = r.minBackwardCycle;
            }

            aggMap.insert(r.rangePercent, a);
        }
        else
        {
            Agg& a = it.value();

            // максимум прямого
            if (r.maxForwardCycle >= 0 &&
                r.maxForwardPosition > a.maxFwdVal)
            {
                a.maxFwdVal = r.maxForwardPosition;
                a.maxFwdCycle = r.maxForwardCycle;
            }

            // минимум обратного
            if (r.minBackwardCycle >= 0 &&
                r.minBackwardPosition < a.minRevVal)
            {
                a.minRevVal = r.minBackwardPosition;
                a.minRevCycle = r.minBackwardCycle;
            }
        }
    }

    constexpr quint16 rowStart = 33, rowStep = 2;

    int i = 0;
    for (auto it = aggMap.begin(); it != aggMap.end(); ++it, ++i)
    {
        quint16 row = rowStart + i * rowStep;
        const Agg& a = it.value();

        writer.cell(m_sheetCyclicTests, row, 2,
                    QString::number(a.rangePercent));

        // forward
        if (a.maxFwdVal != std::numeric_limits<qreal>::lowest()) {
            writer.cell(m_sheetCyclicTests, row, 8,
                        QString::number(a.maxFwdVal, 'f', 2));
            writer.cell(m_sheetCyclicTests, row, 11,
                        QString::number(a.maxFwdCycle + 1));
        } else {
            writer.cell(m_sheetCyclicTests, row, 8, "");
            writer.cell(m_sheetCyclicTests, row, 11, "");
        }

        // reverse
        if (a.minRevVal != std::numeric_limits<qreal>::max()) {
            writer.cell(m_sheetCyclicTests, row, 12,
                        QString::number(a.minRevVal, 'f', 2));
            writer.cell(m_sheetCyclicTests, row, 15,
                        QString::number(a.minRevCycle + 1));
        } else {
            writer.cell(m_sheetCyclicTests, row, 12, "");
            writer.cell(m_sheetCyclicTests, row, 15, "");
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
