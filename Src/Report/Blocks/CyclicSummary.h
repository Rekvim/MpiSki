#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class
        CyclicSummary : public IBlock {
    public:
        enum class CyclicMode {
            Regulatory,
            Shutoff
        };

        struct Layout {
            QString sheet;
            quint16 rowStart;
            quint16 column;
            quint16 rowStep;
        };

        CyclicSummary(Layout layout, CyclicMode mode)
            : m_layout(std::move(layout)), m_mode(mode) {}

        void build(Writer& writer,
                   const Context& ctx) override
        {
            const auto& stroke = ctx.telemetry.stroke;
            const auto& cyclic = ctx.telemetry.cyclicTestRecord;

            quint16 row = m_layout.rowStart;

            QString forward = stroke ? stroke->timeForwardMs : "";
            QString backward = stroke ? stroke->timeBackwardMs : "";

            writer.cell(m_layout.sheet, row, m_layout.column, forward);
            row += m_layout.rowStep;

            writer.cell(m_layout.sheet, row, m_layout.column, backward);
            row += m_layout.rowStep;

            if (m_mode == CyclicMode::Regulatory) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic.numCyclesRegulatory);
                row += m_layout.rowStep;

                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic.sequenceRegulatory);
                row += m_layout.rowStep;

                writer.cell(m_layout.sheet, row, m_layout.column,
                            QTime(0,0)
                                .addSecs(cyclic.totalTimeSecRegulatory)
                                .toString("mm:ss.zzz"));
            } else {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic.numCyclesShutoff);
                row += m_layout.rowStep;

                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic.sequenceShutoff);
                row += m_layout.rowStep;

                writer.cell(m_layout.sheet, row, m_layout.column,
                            QTime(0,0)
                                .addSecs(cyclic.totalTimeSecShutoff)
                                .toString("mm:ss.zzz"));
            }
        }

    private:
        Layout m_layout;
        CyclicMode m_mode;
    };
}