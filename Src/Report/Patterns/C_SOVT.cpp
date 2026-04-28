#include "C_SOVT.h"

#include "Report/Writer.h"
#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/SolenoidDetails.h"
#include "Report/Blocks/CyclicSummary.h"

namespace Report::Patterns {
void C_SOVT::build(
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

    writer.cell(m_sheetCyclicTests, 1, 9,  ctx.valve.positionNumber);

    Blocks::ObjectInfo({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
    Blocks::ValveSpec({m_sheetCyclicTests, 4, 13, false, true}).build(writer, ctx);
    Blocks::CyclicSummary({m_sheetCyclicTests, 20, 8, 2 },
                          Blocks::CyclicSummary::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

    Blocks::SolenoidDetails({m_sheetCyclicTests,
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
}