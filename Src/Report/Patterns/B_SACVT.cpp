#include "B_SACVT.h"
#include "Report/Writer.h"
#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/SolenoidDetails.h"
#include "Report/Blocks/CyclicSummary.h"
#include "Report/Blocks/CyclicRegulatoryRanges.h"

namespace Report::Patterns {
    void B_SACVT::build(
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
        // Страница: 1;
        writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

        Blocks::ObjectInfo({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
        Blocks::ValveSpec({m_sheetCyclicTests, 4, 13, true, true}).build(writer, ctx);
        Blocks::CyclicSummary({m_sheetCyclicTests, 21, 8, 2
                              }, Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

        Report::Blocks::CyclicRegulatoryRanges({m_sheetCyclicTests,
                                                   35, 2, 2, 8, 11, 12, 15
                                               }).build(writer, ctx);

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