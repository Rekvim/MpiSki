#include "StepReaction.h"

#include "Report/Writer.h"
#include "Report/Builder.h"

#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/StepReaction.h"

#include <utility>

namespace Report::Pages {

StepReaction::StepReaction(Layout layout)
    : m_layout(std::move(layout))
{
}

void StepReaction::build(Writer& writer, const Context& ctx)
{
    writer.cell(
        m_layout.sheet,
        m_layout.rowStart,
        9,
        ctx.valve.positionNumber
    );

    Blocks::ObjectInfo({
        m_layout.sheet,
        m_layout.rowStart + 3,
        4
    }).build(writer, ctx);

    Blocks::ValveSpec({
        m_layout.sheet,
        m_layout.rowStart + 3,
        13,
        true,
        m_layout.includeSolenoid
    }).build(writer, ctx);

    Blocks::StepReaction({
        m_layout.sheet,
        m_layout.rowStart + 19,
        2,
        m_layout.rowStart + 56,
        3,
        10
    }).build(writer, ctx);

    writer.cell(
        m_layout.sheet,
        m_layout.rowStart + 68,
        12,
        ctx.params.date
    );
}

}