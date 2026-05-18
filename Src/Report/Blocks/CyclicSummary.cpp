#include "CyclicSummary.h"

#include <QTime>
#include <QDebug>

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

    qDebug() << "Report has stroke:" << ctx.telemetry.testStroke.has_value();

    if (ctx.telemetry.testStroke) {
        qDebug() << "Report stroke:"
                 << ctx.telemetry.testStroke->forwardTimeMs
                 << ctx.telemetry.testStroke->backwardTimeMs;
    }

    qDebug() << "Report has cyclic regulatory:"
             << ctx.telemetry.testСyclicRegulatory.has_value();

    qDebug() << "Report has cyclic shutoff:"
             << ctx.telemetry.testСyclicShutoff.has_value();

    if (const auto& stroke = ctx.telemetry.testStroke) {
        qDebug() << stroke->forwardTimeMs;

        writer.cell(m_layout.sheet, row, m_layout.column,
                    QTime(0, 0).addMSecs(stroke->forwardTimeMs).toString("mm:ss.zzz"));
    }

    row += m_layout.rowStep;

    if (const auto& stroke = ctx.telemetry.testStroke) {
        qDebug() << stroke->backwardTimeMs;

        writer.cell(m_layout.sheet, row, m_layout.column,
                    QTime(0, 0).addMSecs(stroke->backwardTimeMs).toString("mm:ss.zzz"));
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