#include "C_CVT.h"

#include "Report/Writer.h"

#include "Report/Blocks/CyclicSummary.h"

#include "Report/Pages/CyclicRegulatory.h"
#include "Report/Pages/CyclicDeviation.h"
#include "Report/Pages/TechnicalInspection.h"
#include "Report/Pages/StepReaction.h"

namespace Report::Patterns {
void C_CVT::build(
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

        layout.positionRow = 0;

        layout.objectInfoRow = 66;
        layout.valveSpecRow = 66;

        layout.summaryRow = 81;

        layout.deviationRangesRow = 95;

        layout.fioRow = 118;
        layout.dateRow = 122;

        layout.mode = Blocks::CyclicSummary::CyclicMode::Regulatory;

        layout.positionerModel = true;
        layout.includeSolenoid = false;

        Pages::CyclicDeviation(layout).build(writer, ctx);
    }

    {
        Pages::TechnicalInspection::Layout layout;
        layout.sheet = m_sheetTechnicalInspection;

        layout.positionRow = 1;

        layout.objectInfoRow = 5;
        layout.valveSpecRow = 5;

        layout.technicalResultsRow = 26;
        layout.strokeTimeRow = 48;

        layout.firstDateRow = 62;
        layout.fioRow = 70;

        layout.taskImageRow = 80;
        layout.pressureImageRow = 108;
        layout.frictionImageRow = 136;

        layout.secondDateRow = 162;

        layout.positionerModel = true;
        layout.includeSolenoid = false;

        Pages::TechnicalInspection(layout).build(writer, ctx);
    }

    {
        Pages::StepReaction::Layout layout;
        layout.sheet = m_sheetStepReactionTest;

        layout.positionRow = 1;

        layout.objectInfoRow = 4;
        layout.valveSpecRow = 4;

        layout.imageRow = 17;
        layout.tableStartRow = 55;

        layout.dateRow = 67;

        layout.positionerModel = true;
        layout.includeSolenoid = false;

        Pages::StepReaction(layout).build(writer, ctx);
    }

    writer.validation("=ЗИП!$A$1:$A$37", "J50:J59");
    writer.validation("=Заключение!$B$1:$B$4", "E36");
    writer.validation("=Заключение!$C$1:$C$3", "E38");
    writer.validation("=Заключение!$E$1:$E$4", "E40");
    writer.validation("=Заключение!$D$1:$D$5", "E42");
    writer.validation("=Заключение!$F$3", "E44");
}
}