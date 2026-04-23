#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class CyclicRanges : public IBlock {
    public:
        struct Layout {
            QString sheet;
            quint16 rowStart;
            quint16 rowStep;
        };

        explicit CyclicRanges(Layout layout) : m_layout(std::move(layout)) {}

        void build(Writer& w,
                   const Context& ctx) override
        {
            const auto& ranges =
                ctx.telemetry.cyclicTestRecord.regulatoryResult.ranges;

            quint16 row = m_layout.rowStart;

            for (int i = 0; i < ranges.size() && i < 10; ++i) {

                const auto& r = ranges[i];

                w.cell(m_layout.sheet, row, 2,
                       QString::number(r.rangePercent));

                if (r.maxForwardCycle >= 0) {
                    w.cell(m_layout.sheet, row, 8,
                           QString::number(r.maxForwardPosition, 'f', 2));
                    w.cell(m_layout.sheet, row, 11,
                           QString::number(r.maxForwardCycle + 1));
                }

                if (r.minBackwardCycle >= 0) {
                    w.cell(m_layout.sheet, row, 12,
                           QString::number(r.minBackwardPosition, 'f', 2));
                    w.cell(m_layout.sheet, row, 15,
                           QString::number(r.minBackwardCycle + 1));
                }

                row += m_layout.rowStep;
            }
        }

    private:
        Layout m_layout;
    };
}