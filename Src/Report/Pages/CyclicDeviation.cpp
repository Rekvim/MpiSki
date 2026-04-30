#include "CyclicDeviation.h"

#include "Report/Writer.h"
#include "Report/Builder.h"

#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/CyclicSummary.h"
#include "Report/Blocks/PositionSensorDeviationRanges.h"

#include <utility>

namespace Report::Pages {

CyclicDeviation::CyclicDeviation(Layout layout)
    : m_layout(std::move(layout))
{
}

void CyclicDeviation::build(Writer& writer, const Context& ctx)
{
    writer.cell(m_layout.sheet,m_layout.positionRow, m_layout.positionColumn, ctx.valve.positionNumber);

    Blocks::ObjectInfo({
        m_layout.sheet,
        m_layout.objectInfoRow,
        m_layout.objectInfoColumn
    }).build(writer, ctx);

    Blocks::ValveSpec({
        m_layout.sheet,
        m_layout.valveSpecRow,
        m_layout.valveSpecColumn,
        m_layout.positionerModel,
        m_layout.includeSolenoid
    }).build(writer, ctx);

    Blocks::CyclicSummary({
        m_layout.sheet,
        m_layout.summaryRow,
        m_layout.summaryColumn,
        m_layout.summaryRowStep
    }, m_layout.mode).build(writer, ctx);

    Blocks::PositionSensorDeviationRanges({
        m_layout.sheet,
        m_layout.deviationRangesRow,
        m_layout.deviationRangeColumn,
        m_layout.deviationRowStep,
        m_layout.deviationForwardValueColumn,
        m_layout.deviationForwardCycleColumn,
        m_layout.deviationBackwardValueColumn,
        m_layout.deviationBackwardCycleColumn
    }).build(writer, ctx);

    writer.cell(m_layout.sheet, m_layout.fioRow, m_layout.fioColumn, ctx.object.FIO);
    writer.cell(m_layout.sheet, m_layout.dateRow, m_layout.dateColumn, ctx.params.date);
}

}