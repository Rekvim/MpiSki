#include "C_SACVT.h"

#include "Report/Blocks/CyclicSummary.h"

#include "Report/Pages/CyclicRegulatory.h"
#include "Report/Pages/CyclicDeviation.h"
#include "Report/Pages/SolenoidShutoff.h"
#include "Report/Pages/TechnicalInspection.h"
#include "Report/Pages/StepReaction.h"

#include "Report/Writer.h"

namespace Report::Patterns {
    void C_SACVT::build(
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
            layout.valveSpecRow = 4;
            layout.positionerModel = true;
            layout.includeSolenoid = true;

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
            layout.mode = Blocks::CyclicSummary::CyclicMode::Regulatory;

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

        {
            Pages::TechnicalInspection::Layout layout;
            layout.sheet = m_sheetTechnicalInspection;

            layout.positionRow = 1;

            layout.objectInfoRow = 5;
            layout.valveSpecRow = 5;

            layout.technicalResultsRow = 28;
            layout.strokeTimeRow = 50;

            layout.firstDateRow = 64;
            layout.fioRow = 72;

            layout.taskImageRow = 82;
            layout.pressureImageRow = 110;
            layout.frictionImageRow = 138;

            layout.secondDateRow = 164;

            layout.positionerModel = true;
            layout.includeSolenoid = true;

            Pages::TechnicalInspection(layout).build(writer, ctx);
        }

        {
            Pages::StepReaction::Layout layout;
            layout.sheet = m_sheetStepReactionTest;

            layout.positionRow = 1;

            layout.objectInfoRow = 4;
            layout.valveSpecRow = 4;

            layout.imageRow = 19;
            layout.tableStartRow = 57;

            layout.dateRow = 69;

            layout.positionerModel = true;
            layout.includeSolenoid = true;

            Pages::StepReaction(layout).build(writer, ctx);
        }

        writer.validation("=ЗИП!$A$1:$A$37", "J52:J61");
        writer.validation("=Заключение!$B$1:$B$4", "E38");
        writer.validation("=Заключение!$C$1:$C$3", "E40");
        writer.validation("=Заключение!$E$1:$E$4", "E42");
        writer.validation("=Заключение!$D$1:$D$5", "E44");
        writer.validation("=Заключение!$F$3", "E46");
    }
}