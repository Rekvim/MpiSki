#pragma once

#include "IBlock.h"

namespace Report::Blocks {

class CyclicSummary : public IBlock {
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
        : m_layout(std::move(layout))
        , m_mode(mode)
    {}

    void build(Writer& writer, const Context& ctx) override
    {
        quint16 row = m_layout.rowStart;

        if (const auto& stroke = ctx.telemetry.testStroke) {
            writer.cell(m_layout.sheet, row, m_layout.column,
                        stroke->timeForwardMs);
        } else {
            qWarning() << "Report field skipped:"
                       << "CyclicSummary - missing ctx.telemetry.testStroke.timeForwardMs";
        }

        row += m_layout.rowStep;

        if (const auto& stroke = ctx.telemetry.testStroke) {
            writer.cell(m_layout.sheet, row, m_layout.column,
                        stroke->timeBackwardMs);
        } else {
            qWarning() << "Report field skipped:"
                       << "CyclicSummary - missing ctx.telemetry.testStroke.timeBackwardMs";
        }

        row += m_layout.rowStep;

        switch (m_mode) {
        case CyclicMode::Regulatory: {
            const auto& cyclic = ctx.telemetry.testСyclicRegulatory;

            if (cyclic) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic->numCycles);
            } else {
                qWarning() << "Report field skipped:"
                           << "CyclicSummary Regulatory - missing numCycles";
            }

            row += m_layout.rowStep;

            if (cyclic) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic->sequence);
            } else {
                qWarning() << "Report field skipped:"
                           << "CyclicSummary Regulatory - missing sequence";
            }

            row += m_layout.rowStep;

            if (cyclic) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            QTime(0, 0)
                                .addSecs(cyclic->totalTimeSec)
                                .toString("mm:ss.zzz"));
            } else {
                qWarning() << "Report field skipped:"
                           << "CyclicSummary Regulatory - missing totalTimeSec";
            }

            break;
        }

        case CyclicMode::Shutoff: {
            const auto& cyclic = ctx.telemetry.testСyclicShutoff;

            if (cyclic) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic->numCycles);
            } else {
                qWarning() << "Report field skipped:"
                           << "CyclicSummary Shutoff - missing numCycles";
            }

            row += m_layout.rowStep;

            if (cyclic) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            cyclic->sequence);
            } else {
                qWarning() << "Report field skipped:"
                           << "CyclicSummary Shutoff - missing sequence";
            }

            row += m_layout.rowStep;

            if (cyclic) {
                writer.cell(m_layout.sheet, row, m_layout.column,
                            QTime(0, 0)
                                .addSecs(cyclic->totalTimeSec)
                                .toString("mm:ss.zzz"));
            } else {
                qWarning() << "Report field skipped:"
                           << "CyclicSummary Shutoff - missing totalTimeSec";
            }

            break;
        }
        }
    }

private:
    Layout m_layout;
    CyclicMode m_mode;
};

}