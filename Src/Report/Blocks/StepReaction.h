#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class StepReaction: public IBlock {
    public:
        struct Layout {
            QString sheet;

            quint16 imageRow;
            quint16 imageCol;

            quint16 startRow;
            quint16 firstBaseCol;
            quint16 secondBaseCol;
        };

        explicit StepReaction(Layout layout) : m_layout(std::move(layout)) {}

        void build(Writer& writer, const Context& ctx) override
        {
            writer.image(m_layout.sheet, m_layout.imageRow, m_layout.imageCol, ctx.chartStep);

            const auto& results = ctx.telemetry.stepResults;

            auto writeRow = [&](quint16 row, quint16 baseCol, const auto& sr)
            {
                writer.cell(m_layout.sheet, row, baseCol++,
                            QString("%1->%2").arg(sr.from).arg(sr.to));

                writer.cell(m_layout.sheet, row, baseCol++,
                            QTime(0,0)
                                .addMSecs(sr.T_value)
                                .toString("m:ss.zzz"));

                writer.cell(m_layout.sheet, row, baseCol++,
                            QString::number(sr.overshoot, 'f', 2));
            };

            for (int i = 0; i < results.size() && i < 20; ++i)
            {
                const bool firstBlock = i < 10;
                const quint16 baseCol = firstBlock ? m_layout.firstBaseCol
                                                   : m_layout.secondBaseCol;

                quint16 currentRow =  m_layout.startRow + (firstBlock ? i : i - 10);
                writeRow(currentRow, baseCol, results[i]);
            }
        }
    private:
        Layout m_layout;
    };
}