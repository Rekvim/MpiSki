#include "CyclicShutoffPage.h"

#include "Report/Writer.h"
#include "Report/Builder.h"

#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/CyclicSummary.h"

#include <utility>

namespace Report::Pages {

CyclicShutoffPage::CyclicShutoffPage(Layout layout)
    : m_layout(std::move(layout))
{
}

void CyclicShutoffPage::build(Writer& writer, const Context& ctx)
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

    Blocks::CyclicSummary({
        m_layout.sheet,
        m_layout.rowStart + 20,
        8,
        2
    }, Blocks::CyclicSummary::CyclicMode::Shutoff).build(writer, ctx);

    writer.cell(
        m_layout.sheet,
        static_cast<quint16>(m_layout.rowStart + 57),
        4,
        ctx.object.FIO
    );

    writer.cell(
        m_layout.sheet,
        static_cast<quint16>(m_layout.rowStart + 61),
        12,
        ctx.params.date
    );
}

}