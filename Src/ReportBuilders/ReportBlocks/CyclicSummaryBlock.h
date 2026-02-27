#pragma once

#include "IReportBlock.h"

enum class CyclicMode {
    Regulatory,
    Shutoff
};

struct CyclicSummaryLayout {
    QString sheet;
    quint16 rowStart;
    quint16 column;
    quint16 rowStep;
};

class CyclicSummaryBlock : public IReportBlock {
public:
    CyclicSummaryBlock(CyclicSummaryLayout layout,
                       CyclicMode mode)
        : m_layout(std::move(layout)), m_mode(mode) {}

    void build(ReportWriter& writer,
               const ReportContext& ctx) override
    {
        const auto& stroke = ctx.telemetry.strokeTestRecord;
        const auto& cyclic = ctx.telemetry.cyclicTestRecord;

        quint16 row = m_layout.rowStart;

        writer.cell(m_layout.sheet, row, m_layout.column,
                    stroke.timeForwardMs);
        row += m_layout.rowStep;

        writer.cell(m_layout.sheet, row, m_layout.column,
                    stroke.timeBackwardMs);
        row += m_layout.rowStep;

        // Различающиеся поля
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
        }
        else { // Shutoff
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
    CyclicSummaryLayout m_layout;
    CyclicMode m_mode;
};
