#include "C_SOVT.h"

#include "Report/Writer.h"
#include "Report/Pages/SolenoidShutoff.h"

namespace Report::Patterns {
void C_SOVT::build(
    ReportData& report,
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
        Pages::SolenoidShutoff::Layout layout;
        layout.sheet = m_sheetCyclicTests;

        layout.positionRow = 1;
        layout.objectInfoRow = 4;
        layout.valveSpecRow = 4;
        layout.positionerModel = false;
        layout.includeSolenoid = true;

        layout.summaryRow = 20;

        layout.solenoidDetailsRow = 36;
        layout.solenoidSwitch1Row = 33;
        layout.solenoidSwitch2Row = 36;

        layout.fioRow = 53;
        layout.dateRow = 57;

        Pages::SolenoidShutoff(layout).build(writer, ctx);
    }
}
}