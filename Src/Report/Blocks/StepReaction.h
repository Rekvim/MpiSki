#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class StepReaction: public IBlock {
    public:
        struct Layout {
            QString sheet;

            int imageRow;
            int imageCol;

            int startRow;
            int firstBaseCol;
            int secondBaseCol;
        };

        explicit StepReaction(Layout layout) : m_layout(std::move(layout)) {}

        void build(Writer& writer, const Context& ctx) override
        {
            writer.image(m_layout.sheet, m_layout.imageRow, m_layout.imageCol,
                         ctx.chartImages.get(Widgets::Chart::ChartType::Step));
            if (const auto& result = ctx.telemetry.testStep) {
                const auto& steps = result->steps;
                auto writeRow = [&](int row, int baseCol, const auto& sr)
                {
                    writer.cell(m_layout.sheet, row, baseCol++,
                                QString("%1->%2").arg(sr.from).arg(sr.to));

                    const QString time = sr.T_value == 0
                                             ? QObject::tr("Ошибка")
                                             : QTime(0, 0).addMSecs(sr.T_value).toString("m:ss.zzz");

                    writer.cell(m_layout.sheet, row, baseCol++, time);
                    writer.cell(m_layout.sheet, row, baseCol++, sr.overshoot);
                };

                for (int step = 0; step < steps.size() && step < 20; ++step)
                {
                    const bool firstBlock = step < 10;
                    const int baseCol = firstBlock ? m_layout.firstBaseCol
                                                   : m_layout.secondBaseCol;

                    int currentRow = m_layout.startRow + (firstBlock ? step : step - 10);
                    writeRow(currentRow, baseCol, steps.at(step));
                }
            } else {
                qWarning() << "Report block skipped: ctx.telemetry.testStep";
            }
        }
    private:
        Layout m_layout;
    };
}