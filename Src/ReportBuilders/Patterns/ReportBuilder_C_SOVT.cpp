#include "ReportBuilder_C_SOVT.h"
#include "Src/ReportBuilders/ReportWriter.h"
#include "Src/ReportBuilders/ReportBlocks/ObjectInfoBlock.h"
#include "Src/ReportBuilders/ReportBlocks/ValveSpecBlock.h"
#include "Src/ReportBuilders/ReportBlocks/SolenoidDetailsBlock.h"
#include "Src/ReportBuilders/ReportBlocks/CyclicSummaryBlock.h"

ReportBuilder_C_SOVT::ReportBuilder_C_SOVT() {}

void ReportBuilder_C_SOVT::buildReport(
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

    writer.cell(m_sheetCyclicTests, 1, 9,  ctx.valve.positionNumber);

    ObjectInfoBlock({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 4, 13, false, true}).build(writer, ctx);
    CyclicSummaryBlock({m_sheetCyclicTests,
                        20,
                        8,
                        2
                       }, CyclicMode::Shutoff).build(writer, ctx);

    SolenoidDetailsBlock({m_sheetCyclicTests,
                             36, // rowBase
                             8, // colCount
                             10, // colOn
                             13, // colOff
                             2, // rowStep
                             44, // rowSwitch1
                             46 // rowSwitch2
                         }).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 53, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 57, 12, ctx.params.date);
}
