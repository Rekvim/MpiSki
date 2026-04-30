#include "B_CVT.h"

#include "Report/Writer.h"

#include "Report/Pages/CyclicRegulatory.h"
#include "Report/Pages/CyclicDeviation.h"

namespace Report::Patterns {

void B_CVT::build(
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

    {
        Pages::CyclicRegulatory::Layout layout;
        layout.sheet = m_sheetCyclicTests;

        layout.positionRow = 1;

        layout.objectInfoRow = 4;
        layout.valveSpecRow = 4;

        layout.summaryRow = 19;

        layout.rangesRow = 33;

        layout.fioRow = 56;
        layout.dateRow = 60;

        layout.positionerModel = true;
        layout.includeSolenoid = false;

        Pages::CyclicRegulatory(layout).build(writer, ctx);
    }

    {
        Pages::CyclicDeviation::Layout layout;
        layout.sheet = m_sheetCyclicTests;

        layout.objectInfoRow = 66;
        layout.valveSpecRow = 66;

        layout.summaryRow = 81;

        layout.deviationRangesRow = 95;
        layout.deviationRangesRow = 99;

        layout.fioRow = 118;
        layout.dateRow = 122;

        layout.mode = Blocks::CyclicSummary::CyclicMode::Regulatory;

        layout.positionerModel = false;
        layout.includeSolenoid = false;

        Pages::CyclicDeviation(layout).build(writer, ctx);
    }
}

}