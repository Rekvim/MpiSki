#include "C_SACVT.h"
#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/CyclicSummary.h"
#include "Report/Blocks/SolenoidDetails.h"
#include "Report/Blocks/CyclicRanges.h"
#include "Report/Blocks/StepReaction.h"
#include "Report/Blocks/TechnicalResults.h"
#include "Report/Blocks/CyclicRegulatoryRanges.h"

namespace Report::Patterns {
    void C_SACVT::build(
        Saver::Report& report,
        const Telemetry& telemetryStore,
        const ObjectInfo& objectInfo,
        const ValveInfo& valveInfo,
        const OtherParameters& otherParams,
        const ChartImageStorage& chartImages
        )
    {
        Writer writer(report);

        Context ctx{
            telemetryStore,
            objectInfo,
            valveInfo,
            otherParams,
            chartImages
        };

        writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetCyclicTests, 4, 4}).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 4, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests,
                               21,
                               8,
                               2
                           }, Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

        Report::Blocks::CyclicRegulatoryRanges({m_sheetCyclicTests,
                                                   35, 2, 2, 8, 11, 12, 15
                                               }).build(writer, ctx);

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

        writer.image(m_sheetTechnicalInspection, 82, 1,
                     ctx.chartImages.get(Widgets::Chart::ChartType::Task));

        writer.image(m_sheetTechnicalInspection, 110, 1,
                     ctx.chartImages.get(Widgets::Chart::ChartType::Pressure));

        writer.image(m_sheetTechnicalInspection, 138, 1,
                     ctx.chartImages.get(Widgets::Chart::ChartType::Friction));

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