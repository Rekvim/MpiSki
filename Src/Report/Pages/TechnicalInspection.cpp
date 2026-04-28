#include "TechnicalInspection.h"

#include "Report/Writer.h"
#include "Report/Builder.h"

#include "Report/Blocks/ObjectInfo.h"
#include "Report/Blocks/ValveSpec.h"
#include "Report/Blocks/TechnicalResults.h"

#include "Widgets/Chart/ChartType.h"

#include <utility>

namespace Report::Pages {

TechnicalInspection::TechnicalInspection(Layout layout)
    : m_layout(std::move(layout))
{
}

void TechnicalInspection::build(Writer& writer, const Context& ctx)
{
    writer.cell(
        m_layout.sheet,
        m_layout.rowStart,
        9,
        ctx.valve.positionNumber
    );

    Blocks::ObjectInfo({
        m_layout.sheet,
        m_layout.rowStart + 4,
        4
    }).build(writer, ctx);

    Blocks::ValveSpec({
        m_layout.sheet,
        m_layout.rowStart + 4,
        13,
        true,
        m_layout.includeSolenoid
    }).build(writer, ctx);

    Blocks::TechnicalResults({
        m_layout.sheet,
        m_layout.rowStart + 27,
        5,
        8,
        11,
        m_layout.rowStart + 49
    }).build(writer, ctx);

    writer.cell(
        m_layout.sheet,
        m_layout.rowStart + 63,
        12,
        ctx.params.date
    );

    writer.cell(
        m_layout.sheet,
        m_layout.rowStart + 71,
        4,
        ctx.object.FIO
    );

    writer.image(
        m_layout.sheet,
        m_layout.rowStart + 81,
        1,
        ctx.chartImages.get(Widgets::Chart::ChartType::Task)
    );

    writer.image(
        m_layout.sheet,
        m_layout.rowStart + 109,
        1,
        ctx.chartImages.get(Widgets::Chart::ChartType::Pressure)
    );

    writer.image(
        m_layout.sheet,
        m_layout.rowStart + 137,
        1,
        ctx.chartImages.get(Widgets::Chart::ChartType::Friction)
    );

    writer.cell(
        m_layout.sheet,
        m_layout.rowStart + 163,
        12,
        ctx.params.date
    );
}

}