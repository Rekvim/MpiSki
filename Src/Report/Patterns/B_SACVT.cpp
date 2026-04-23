#include "B_SACVT.h"
#include "Report/Writer.h"
#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/SolenoidDetails.h"
#include "Report/Blocks/CyclicSummary.h"

namespace Report::Patterns {
    void B_SACVT::buildReport(
        Saver::Report& report,
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
        Writer writer(report);

        Context ctx{
            telemetryStore,
            objectInfo,
            valveInfo,
            otherParams,
            imageChartTask,
            imageChartPressure,
            imageChartFriction,
            imageChartStep
        };
        // Страница: 1;
        writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 4, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests,
                               21,
                               8,
                               2
                           }, Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

        // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
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

        constexpr quint16 rowStart = 35, rowStep = 2;

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

        writer.cell(m_sheetCyclicTests, 58, 4, ctx.object.FIO);
        writer.cell(m_sheetCyclicTests, 62, 12, ctx.params.date);

        // Страница: 2;
        Blocks::ObjectInfo({m_sheetCyclicTests, 68, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 68, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests, 85, 8, 2 },
                             Blocks::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

        writer.cell(m_sheetCyclicTests, 122, 4, objectInfo.FIO);
        writer.cell(m_sheetCyclicTests, 126, 12, otherParams.date);

        //Страница: 3;
        Blocks::ObjectInfo({m_sheetCyclicTests, 131, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 131, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests, 148, 8, 2},
                             Blocks::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

        Blocks::SolenoidDetails({m_sheetCyclicTests,
                                 164, // rowBase
                                 8, // colCount
                                 10, // colOn
                                 13, // colOff
                                 2, // rowStep
                                 172, // rowSwitch1
                                 174 // rowSwitch2
                             }).build(writer, ctx);

        writer.cell(m_sheetCyclicTests, 181, 4, ctx.object.FIO);
        writer.cell(m_sheetCyclicTests, 185, 12, ctx.params.date);
    }
}