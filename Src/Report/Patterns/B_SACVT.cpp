#include "B_SACVT.h"
#include "Report/Writer.h"

#include "Report/Blocks/CyclicSummary.h"

#include "Report/Pages/CyclicRegulatory.h"
#include "Report/Pages/CyclicDeviation.h"
#include "Report/Pages/SolenoidShutoff.h"

namespace Report::Patterns {
    void B_SACVT::build(
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
            Pages::CyclicRegulatory::Layout layout;
            layout.sheet = m_sheetCyclicTests;
            layout.positionRow = 1;

            layout.objectInfoRow = 4;
            layout.positionerModel = true;
            layout.includeSolenoid = true;

            layout.valveSpecRow = 4;
            layout.summaryRow = 21;
            layout.rangesRow = 35;
            layout.fioRow = 58;
            layout.dateRow = 62;

            Pages::CyclicRegulatory(layout).build(writer, ctx);
        }

        {
            Pages::CyclicDeviation::Layout layout;
            layout.sheet = m_sheetCyclicTests;

            layout.positionRow = 65;
            layout.objectInfoRow = 68;

            layout.valveSpecRow = 68;
            layout.positionerModel = true;
            layout.includeSolenoid = true;

            layout.summaryRow = 85;
            layout.mode = Blocks::CyclicSummary::CyclicMode::Shutoff;

            layout.deviationRangesRow = 99;

            layout.fioRow = 122;
            layout.dateRow = 126;

            Pages::CyclicDeviation(layout).build(writer, ctx);
        }

        {
            Pages::SolenoidShutoff::Layout layout;
            layout.sheet = m_sheetCyclicTests;

            layout.positionRow = 128;
            layout.objectInfoRow = 131;
            layout.valveSpecRow = 131;
            layout.positionerModel = true;
            layout.includeSolenoid = true;

            layout.summaryRow = 148;

            layout.solenoidDetailsRow = 164;
            layout.solenoidSwitch1Row = 172;
            layout.solenoidSwitch2Row = 174;


            layout.fioRow = 181;
            layout.dateRow = 185;

            Pages::SolenoidShutoff(layout).build(writer, ctx);
        }
    }
}