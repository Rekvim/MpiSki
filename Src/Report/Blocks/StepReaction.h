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
            if (const auto& result = ctx.telemetry.testStep) {
                const auto& steps = result->steps;
                auto writeRow = [&](quint16 row, quint16 baseCol, const auto& sr)
                {
                    writer.cell(m_layout.sheet, row, baseCol++,
                                QString("%1->%2").arg(sr.from).arg(sr.to));

                    const QString time = sr.T_value == 0
                                             ? QObject::tr("Ошибка")
                                             : QTime(0, 0).addMSecs(sr.T_value).toString("m:ss.zzz");

                    writer.cell(m_layout.sheet, row, baseCol++, time);
                    writer.cell(m_layout.sheet, row, baseCol++, sr.overshoot);
                };

                for (int i = 0; i < steps.size() && i < 20; ++i)
                {
                    const bool firstBlock = i < 10;
                    const quint16 baseCol = firstBlock ? m_layout.firstBaseCol
                                                       : m_layout.secondBaseCol;

                    quint16 currentRow = m_layout.startRow + (firstBlock ? i : i - 10);
                    writeRow(currentRow, baseCol, steps.at(i));
                }
            } else {
                qWarning() << "Report block skipped: ctx.telemetry.testStep";
            }
        }
    private:
        Layout m_layout;
    };
}