#include "C_CVT.h"

#include "Report/Writer.h"
#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/CyclicSummary.h"
#include "Report/Blocks/CyclicRanges.h"
#include "Report/Blocks/StepReaction.h"
#include "Report/Blocks/TechnicalResults.h"
#include "Report/Blocks/CyclicRegulatoryRanges.h"

namespace Report::Patterns {
void C_CVT::buildReport(
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

    Blocks::ObjectInfo({m_sheetCyclicTests, 4, 4}).build(writer, ctx);
    Blocks::ValveSpec({m_sheetCyclicTests, 4, 13, true, false}).build(writer, ctx);
    Blocks::CyclicSummary({m_sheetCyclicTests, 19, 8, 2 },
                       Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

    Report::Blocks::CyclicRegulatoryRanges({m_sheetCyclicTests,
                                               33, 2, 2, 8, 11, 12, 15
                                           }).build(writer, ctx);

    // Blocks::CyclicRanges({m_sheetCyclicTests,
    //                              33,
    //                              2
    //                          }).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 56, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 60, 12, ctx.params.date);

    // Страница: 2;
    Blocks::ObjectInfo({m_sheetCyclicTests, 66, 4}).build(writer, ctx);
    Blocks::ValveSpec({m_sheetCyclicTests, 66, 13, true, false}).build(writer, ctx);
    Blocks::CyclicSummary({m_sheetCyclicTests,
                           81,
                           8,
                           2
                       }, Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 118, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 122, 12, ctx.params.date);

    // Страница: 1;
    writer.cell(m_sheetTechnicalInspection, 1, 9, valveInfo.positionNumber);
    Blocks::ObjectInfo({m_sheetTechnicalInspection, 5, 4}).build(writer, ctx);
    Blocks::ValveSpec({m_sheetTechnicalInspection, 5, 13, true, false}).build(writer, ctx);

    Blocks::TechnicalResults({m_sheetTechnicalInspection,
                              26, // rowStart
                              5, // colFact
                              8, // colNorm
                              11, // colResult
                              48 // rowStrokeTime
                             }).build(writer, ctx);

    writer.cell(m_sheetTechnicalInspection, 62, 12, ctx.params.date);
    writer.cell(m_sheetTechnicalInspection, 70, 4, ctx.object.FIO);

    writer.image(m_sheetTechnicalInspection, 80, 1, imageChartTask);
    writer.image(m_sheetTechnicalInspection, 108, 1, imageChartPressure);
    writer.image(m_sheetTechnicalInspection, 136, 1, imageChartFriction);

    writer.cell( m_sheetTechnicalInspection, 162, 12, ctx.params.date);

    // Страница: 1;
    writer.cell(m_sheetStepReactionTest, 1, 9, valveInfo.positionNumber);

    Blocks::ObjectInfo({m_sheetStepReactionTest, 4, 4}).build(writer, ctx);
    Blocks::ValveSpec({m_sheetStepReactionTest, 4, 13, true, false}).build(writer, ctx);
    Blocks::StepReaction({m_sheetStepReactionTest,
                          18,  // imageRow
                          2,   // imageCol
                          55,  // startRow
                          3,   // firstBaseCol
                          10   // secondBaseCol
                      }).build(writer, ctx);

    writer.cell(m_sheetStepReactionTest, 67, 12, ctx.params.date);

    writer.validation("=ЗИП!$A$1:$A$37", "J50:J59");
    writer.validation("=Заключение!$B$1:$B$4", "E36");
    writer.validation("=Заключение!$C$1:$C$3", "E38");
    writer.validation("=Заключение!$E$1:$E$4", "E40");
    writer.validation("=Заключение!$D$1:$D$5", "E42");
    writer.validation("=Заключение!$F$3", "E44");
}
}