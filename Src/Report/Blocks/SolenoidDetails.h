#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class SolenoidDetails : public IBlock {
    public:
        struct Layout {
            QString sheet;

            quint16 rowBase;
            quint16 colCount;
            quint16 colOn;
            quint16 colOff;
            quint16 rowStep;

            quint16 rowSwitch1;
            quint16 rowSwitch2;
        };

        explicit SolenoidDetails(Layout layout)
            : m_layout(std::move(layout)) {}

        void build(Writer& w,
                   const Context& ctx) override {
            {
                const auto& cyclic = ctx.telemetry.cyclicTestRecord;

                w.cell(m_layout.sheet, m_layout.rowBase, m_layout.colCount, cyclic.numCyclesShutoff);
                w.cell(m_layout.sheet, m_layout.rowBase + m_layout.rowStep, m_layout.colCount, cyclic.numCyclesShutoff);

                const auto& ons = cyclic.doOnCounts;
                const auto& offs = cyclic.doOffCounts;

                for (int i = 0; i < ons.size(); ++i) {
                    if (ons[i] == 0 && offs.value(i, 0) == 0)
                        continue;

                    quint16 row = m_layout.rowBase + quint16(i) * m_layout.rowStep;

                    w.cell(m_layout.sheet, row, m_layout.colOn,  ons[i]);
                    w.cell(m_layout.sheet, row, m_layout.colOff, offs.value(i, 0));
                }

                w.cell(m_layout.sheet, m_layout.rowSwitch1, m_layout.colCount, cyclic.numCyclesShutoff);
                w.cell(m_layout.sheet, m_layout.rowSwitch1, m_layout.colOn, cyclic.switch3to0Count);
                w.cell(m_layout.sheet, m_layout.rowSwitch1, m_layout.colOff, cyclic.switch0to3Count);

                w.cell(m_layout.sheet, m_layout.rowSwitch2, m_layout.colCount, cyclic.numCyclesShutoff);
                w.cell(m_layout.sheet, m_layout.rowSwitch2, m_layout.colOn, cyclic.switch0to3Count);
                w.cell(m_layout.sheet, m_layout.rowSwitch2, m_layout.colOff, cyclic.switch3to0Count);
            }
        }

    private:
        Layout m_layout;
    };
}