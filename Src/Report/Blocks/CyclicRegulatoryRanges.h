#pragma once

#include "IBlock.h"

#include <QMap>
#include <QDebug>
#include <limits>

namespace Report::Blocks {

class CyclicRegulatoryRanges : public IBlock {
public:
    struct Layout {
        QString sheet;

        quint16 rowStart = 0;
        quint16 rowStep = 1;

        quint16 rangeCol = 0;

        quint16 forwardValueCol = 0;
        quint16 forwardCycleCol = 0;

        quint16 reverseValueCol = 0;
        quint16 reverseCycleCol = 0;
    };

    explicit CyclicRegulatoryRanges(Layout layout) : m_layout(std::move(layout)) {}

    void build(Writer& writer, const Context& ctx) override
    {
        if (const auto& result = ctx.telemetry.testСyclicRegulatory) {
            const auto& ranges = result->ranges;

            struct Agg {
                qreal rangePercent = 0.0;

                qreal maxFwdVal = std::numeric_limits<qreal>::lowest();
                int maxFwdCycle = -1;

                qreal minRevVal = std::numeric_limits<qreal>::max();
                int minRevCycle = -1;
            };

            QMap<qreal, Agg> aggMap;

            for (const auto& r : ranges)
            {
                auto it = aggMap.find(r.rangePercent);

                if (it == aggMap.end())
                {
                    Agg a;
                    a.rangePercent = r.rangePercent;

                    if (r.maxForwardCycle >= 0) {
                        a.maxFwdVal = r.maxForwardPosition;
                        a.maxFwdCycle = r.maxForwardCycle;
                    }

                    if (r.minBackwardCycle >= 0) {
                        a.minRevVal = r.minBackwardPosition;
                        a.minRevCycle = r.minBackwardCycle;
                    }

                    aggMap.insert(r.rangePercent, a);
                }
                else
                {
                    Agg& a = it.value();

                    if (r.maxForwardCycle >= 0 &&
                        r.maxForwardPosition > a.maxFwdVal)
                    {
                        a.maxFwdVal = r.maxForwardPosition;
                        a.maxFwdCycle = r.maxForwardCycle;
                    }

                    if (r.minBackwardCycle >= 0 &&
                        r.minBackwardPosition < a.minRevVal)
                    {
                        a.minRevVal = r.minBackwardPosition;
                        a.minRevCycle = r.minBackwardCycle;
                    }
                }
            }

            int i = 0;

            for (auto it = aggMap.begin(); it != aggMap.end(); ++it, ++i)
            {
                const quint16 row = m_layout.rowStart + i * m_layout.rowStep;
                const Agg& a = it.value();

                writer.cell(m_layout.sheet, row, m_layout.rangeCol,
                            QString::number(a.rangePercent));

                if (a.maxFwdCycle >= 0) {
                    writer.cell(m_layout.sheet, row, m_layout.forwardValueCol,
                                QString::number(a.maxFwdVal, 'f', 2));

                    writer.cell(m_layout.sheet, row, m_layout.forwardCycleCol,
                                QString::number(a.maxFwdCycle + 1));
                } else {
                    writer.cell(m_layout.sheet, row, m_layout.forwardValueCol, "");
                    writer.cell(m_layout.sheet, row, m_layout.forwardCycleCol, "");
                }

                if (a.minRevCycle >= 0) {
                    writer.cell(m_layout.sheet, row, m_layout.reverseValueCol,
                                QString::number(a.minRevVal, 'f', 2));

                    writer.cell(m_layout.sheet, row, m_layout.reverseCycleCol,
                                QString::number(a.minRevCycle + 1));
                } else {
                    writer.cell(m_layout.sheet, row, m_layout.reverseValueCol, "");
                    writer.cell(m_layout.sheet, row, m_layout.reverseCycleCol, "");
                }
            }
        } else {
            qWarning() << "Report block skipped: CyclicRegulatoryRanges";
        }
    }

private:
    Layout m_layout;
};

}