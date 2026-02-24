#pragma once
#include "IReportBlock.h"

struct StrokeSummaryLayout {
    QString sheet;
    quint16 rowStart;   // строка первого параметра
    quint16 column;     // колонка (обычно 8)
    quint16 rowStep;    // шаг между строками (обычно 2)
};

class StrokeSummaryBlock : public IReportBlock {
public:
    explicit StrokeSummaryBlock(StrokeSummaryLayout layout)
        : m_layout(layout) {}

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

        writer.cell(m_layout.sheet, row, m_layout.column,
                    QString::number(cyclic.numCyclesRegulatory));
        row += m_layout.rowStep;

        writer.cell(m_layout.sheet, row, m_layout.column,
                    cyclic.sequenceRegulatory);
        row += m_layout.rowStep;

        writer.cell(m_layout.sheet, row, m_layout.column,
                    QTime(0,0)
                        .addSecs(cyclic.totalTimeSecRegulatory)
                        .toString("mm:ss.zzz"));
    }

private:
    StrokeSummaryLayout m_layout;
};
