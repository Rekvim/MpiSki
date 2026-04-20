#include "C_SACVT.h"
#include "Src/Report/Blocks/ObjectInfo.h"
#include "Src/Report/Blocks/ValveSpec.h"
#include "Src/Report/Blocks/CyclicSummary.h"
#include "Src/Report/Blocks/SolenoidDetails.h"
#include "Src/Report/Blocks/CyclicRanges.h"
#include "Src/Report/Blocks/StepReaction.h"
#include "Src/Report/Blocks/TechnicalResults.h"

namespace Report::Patterns {
    void C_SACVT::buildReport(
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

        writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetCyclicTests, 4, 4}).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 4, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests,
                               21,
                               8,
                               2
                           }, Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

        // Лист: Отчет ЦТ; Страница: 1 Блок: Циклические испытания позиционера
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

        // CyclicRangesBlock({m_sheetCyclicTests,
        //                              35,
        //                              2
        //                          }).build(writer, ctx);

        writer.cell(m_sheetCyclicTests, 62, 11, ctx.params.date);

        // Страница: 2
        writer.cell(m_sheetCyclicTests, 65, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetCyclicTests, 68, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 68, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests,
                               85,
                               8,
                               2
                           }, Blocks::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

        writer.cell(m_sheetCyclicTests, 122, 4, ctx.object.FIO);
        writer.cell(m_sheetCyclicTests, 126, 12, ctx.params.date);

        // Страница: 3
        writer.cell(m_sheetCyclicTests, 128, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetCyclicTests, 131, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 131, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests,
                               148,
                               8,
                               2
                           }, Blocks::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

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

        // Страница: 1;
        writer.cell(m_sheetTechnicalInspection, 1, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetTechnicalInspection, 5, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetTechnicalInspection, 5, 13, true, true}).build(writer, ctx);

        Blocks::TechnicalResults({m_sheetTechnicalInspection,
                                  28, // rowStart
                                  5, // colFact
                                  8, // colNorm
                                  11, // colResult
                                  50 // rowStrokeTime
                              }).build(writer, ctx);

        writer.cell(m_sheetTechnicalInspection, 64, 12, ctx.params.date);
        writer.cell(m_sheetTechnicalInspection, 72, 4, ctx.object.FIO);

        writer.image(m_sheetTechnicalInspection, 82, 1, ctx.chartTask);
        writer.image(m_sheetTechnicalInspection, 110, 1, ctx.chartPressure);
        writer.image(m_sheetTechnicalInspection, 138, 1, ctx.chartFriction);

        writer.cell(m_sheetTechnicalInspection, 164, 12, ctx.params.date);

        // Страница: 1;
        writer.cell(m_sheetStepReactionTest, 1, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetStepReactionTest, 4, 4}).build(writer, ctx);
        Blocks::ValveSpec({m_sheetStepReactionTest, 4, 13, true, true}).build(writer, ctx);

        // writer.image(m_sheetStepReactionTest, 20, 2, imageChartStep);

        Blocks::StepReaction({m_sheetStepReactionTest,
                              20,  // imageRow
                              2,   // imageCol
                              57,  // startRow
                              3,   // firstBaseCol
                              10   // secondBaseCol
                             }).build(writer, ctx);

        writer.cell(m_sheetStepReactionTest, 69, 12, ctx.params.date);

        writer.validation("=ЗИП!$A$1:$A$37", "J52:J61");
        writer.validation("=Заключение!$B$1:$B$4", "E38");
        writer.validation("=Заключение!$C$1:$C$3", "E40");
        writer.validation("=Заключение!$E$1:$E$4", "E42");
        writer.validation("=Заключение!$D$1:$D$5", "E44");
        writer.validation("=Заключение!$F$3", "E46");
    }
}