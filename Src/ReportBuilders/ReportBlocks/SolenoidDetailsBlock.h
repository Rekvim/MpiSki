#pragma once

#include "IReportBlock.h"

struct SolenoidDetailsLayout {
    QString sheet;

    quint16 rowBase;     // 36
    quint16 colCount;    // 8
    quint16 colOn;       // 10
    quint16 colOff;      // 13
    quint16 rowStep;     // 2

    quint16 rowSwitch1;  // 44
    quint16 rowSwitch2;  // 46
};

class SolenoidDetailsBlock : public IReportBlock {
public:
    explicit SolenoidDetailsBlock(SolenoidDetailsLayout l)
        : m(l) {}

    void build(ReportWriter& w,
               const ReportContext& ctx) override {
        {
            const auto& cyclic = ctx.telemetry.cyclicTestRecord;

            w.cell(m.sheet, m.rowBase, m.colCount, cyclic.numCyclesShutoff);
            w.cell(m.sheet, m.rowBase + m.rowStep, m.colCount, cyclic.numCyclesShutoff);

            const auto& ons  = cyclic.doOnCounts;
            const auto& offs = cyclic.doOffCounts;

            for (int i = 0; i < ons.size(); ++i) {
                if (ons[i] == 0 && offs.value(i, 0) == 0)
                    continue;

                quint16 row = m.rowBase + quint16(i) * m.rowStep;

                w.cell(m.sheet, row, m.colOn,  ons[i]);
                w.cell(m.sheet, row, m.colOff, offs.value(i, 0));
            }

            w.cell(m.sheet, m.rowSwitch1, m.colCount, cyclic.numCyclesShutoff);
            w.cell(m.sheet, m.rowSwitch1, m.colOn, cyclic.switch3to0Count);
            w.cell(m.sheet, m.rowSwitch1, m.colOff, cyclic.switch0to3Count);

            w.cell(m.sheet, m.rowSwitch2, m.colCount, cyclic.numCyclesShutoff);
            w.cell(m.sheet, m.rowSwitch2, m.colOn, cyclic.switch0to3Count);
            w.cell(m.sheet, m.rowSwitch2, m.colOff, cyclic.switch3to0Count);
        }
    }

private:
    SolenoidDetailsLayout m;
};
