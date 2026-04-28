#include "CyclicRegulatory.h"

#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/CyclicSummary.h"
#include "Report/Blocks/CyclicRegulatoryRanges.h"

#include "Report/Writer.h"

#include <utility>

namespace Report::Pages {

CyclicRegulatory::CyclicRegulatory(Layout layout)
    : m_layout(std::move(layout))
{
}

void CyclicRegulatory::build(Writer& writer, const Context& ctx)
{
    writer.cell(m_layout.sheet, 1, 9, ctx.valve.positionNumber);

    Blocks::ObjectInfo({
        m_layout.sheet,
        4,
        4
    }).build(writer, ctx);

    Blocks::ValveSpec({
        m_layout.sheet,
        4,
        13,
        true,
        m_layout.includeSolenoid
    }).build(writer, ctx);

    Blocks::CyclicSummary({
        m_layout.sheet,
        19,
        8,
        2
    }, Blocks::CyclicSummary::CyclicMode::Regulatory).build(writer, ctx);

    Blocks::CyclicRegulatoryRanges({
        m_layout.sheet,
        33,
        2,
        2,
        8,
        11,
        12,
        15
    }).build(writer, ctx);

    writer.cell(m_layout.sheet, 60, 11, ctx.params.date);
}
}