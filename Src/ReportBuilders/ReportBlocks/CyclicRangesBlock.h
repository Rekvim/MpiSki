#pragma once

#include "IReportBlock.h"

struct RegulatoryDeviationLayout {
    QString sheet;
    quint16 rowStart;
    quint16 rowStep;
};

class CyclicRangesBlock : public IReportBlock {
public:
    explicit CyclicRangesBlock(RegulatoryDeviationLayout layout)
        : m_layout(std::move(layout)) {}

    void build(ReportWriter& w,
               const ReportContext& ctx) override
    {
        const auto& ranges =
            ctx.telemetry.cyclicTestRecord.ranges;

        quint16 row = m_layout.rowStart;

        for (int i = 0; i < ranges.size() && i < 10; ++i) {

            const auto& r = ranges[i];

            w.cell(m_layout.sheet, row, 2,
                   QString::number(r.rangePercent));

            if (r.maxForwardCycle >= 0) {
                w.cell(m.sheet, row, 8,
                       QString::number(r.maxForwardValue, 'f', 2));
                w.cell(m.sheet, row, 11,
                       QString::number(r.maxForwardCycle + 1));
            }

            if (r.maxReverseCycle >= 0) {
                w.cell(m.sheet, row, 12,
                       QString::number(r.maxReverseValue, 'f', 2));
                w.cell(m.sheet, row, 15,
                       QString::number(r.maxReverseCycle + 1));
            }

            row += m_layout.rowStep;
        }
    }

private:
    RegulatoryDeviationLayout m_layout;
};
