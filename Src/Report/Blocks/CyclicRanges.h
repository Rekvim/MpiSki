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

        void build(Writer& w, const Context& ctx) override {
            if (ctx.telemetry.testСyclicRegulatory) {
                const auto& ranges = ctx.telemetry.testСyclicRegulatory->ranges;

                quint16 row = m_layout.rowStart;

                for (int i = 0; i < ranges.size() && i < 10; ++i) {
                    const auto& r = ranges[i];
                    w.cell(m_layout.sheet, row, 2, r.rangePercent);

                    if (r.maxForwardCycle >= 0) {
                        w.cell(m_layout.sheet, row, 8, r.maxForwardPosition);
                        w.cell(m_layout.sheet, row, 11, r.maxForwardCycle + 1);
                    } if (r.minBackwardCycle >= 0) {
                        w.cell(m_layout.sheet, row, 12, r.minBackwardPosition);
                        w.cell(m_layout.sheet, row, 15, r.minBackwardCycle + 1);
                    }

                    row += m_layout.rowStep;
                }
            } else {
                qWarning() << "Report block skipped: ctx.telemetry.testСyclicRegulatory";
            }
        }

    private:
        Layout m_layout;
    };
}