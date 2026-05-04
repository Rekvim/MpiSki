#include "CyclicSummary.h"

#include <QTime>

#include "Report/Writer.h"
#include "Report/Builder.h"

namespace Report::Blocks {

CyclicSummary::CyclicSummary(Layout layout, CyclicMode mode)
    : m_layout(std::move(layout))
    , m_mode(mode)
{}

void CyclicSummary::build(Writer& writer, const Context& ctx)
{
    int row = m_layout.rowStart;

    if (const auto& stroke = ctx.telemetry.testStroke) {
        writer.cell(m_layout.sheet, row, m_layout.column,
                    stroke->timeForwardMs);
    }

    row += m_layout.rowStep;

    if (const auto& stroke = ctx.telemetry.testStroke) {
        writer.cell(m_layout.sheet, row, m_layout.column,
                    stroke->timeBackwardMs);
    }

    row += m_layout.rowStep;

    switch (m_mode) {
    case CyclicMode::Regulatory: {
        const auto& cyclic = ctx.telemetry.testСyclicRegulatory;

        if (cyclic) {
            writer.cell(m_layout.sheet, row, m_layout.column, cyclic->numCycles);
            row += m_layout.rowStep;

            writer.cell(m_layout.sheet, row, m_layout.column, cyclic->sequence);
            row += m_layout.rowStep;

            writer.cell(m_layout.sheet, row, m_layout.column,
                        QTime(0, 0)
                            .addSecs(cyclic->totalTimeSec)
                            .toString("mm:ss.zzz"));
        } break;
    }
    case CyclicMode::Shutoff: {
        const auto& cyclic = ctx.telemetry.testСyclicShutoff;

        if (cyclic) {
            writer.cell(m_layout.sheet, row, m_layout.column, cyclic->numCycles);
            row += m_layout.rowStep;

            writer.cell(m_layout.sheet, row, m_layout.column, cyclic->sequence);
            row += m_layout.rowStep;

            writer.cell(m_layout.sheet, row, m_layout.column,
                        QTime(0, 0)
                            .addSecs(cyclic->totalTimeSec)
                            .toString("mm:ss.zzz"));
        } break;
    }
    }
}
}