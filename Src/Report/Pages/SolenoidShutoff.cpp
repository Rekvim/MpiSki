#include "SolenoidShutoff.h"

#include "Report/Writer.h"
#include "Report/Builder.h"

#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/CyclicSummary.h"
#include "Report/Blocks/SolenoidDetails.h"

#include <utility>

namespace Report::Pages {

SolenoidShutoff::SolenoidShutoff(Layout layout)
    : m_layout(std::move(layout))
{
}

void SolenoidShutoff::build(Writer& writer, const Context& ctx)
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
        true
    }).build(writer, ctx);

    Blocks::CyclicSummary({
        m_layout.sheet,
        m_layout.rowStart + 20,
        8,
        2
    }, Blocks::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

    Blocks::SolenoidDetails({
        m_layout.sheet,
        m_layout.rowStart + 36,
        8,
        10,
        13,
        2,
        m_layout.rowStart + 44,
        m_layout.rowStart + 46
    }).build(writer, ctx);

    writer.cell(
        m_layout.sheet,
        m_layout.rowStart + 53,
        4,
        ctx.object.FIO
    );

    writer.cell(
        m_layout.sheet,
        m_layout.rowStart + 57,
        12,
        ctx.params.date
    );
}

}