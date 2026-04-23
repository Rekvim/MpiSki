#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class TechnicalResults : public IBlock {
    public:
        struct Layout {
            QString sheet;
            quint16 rowStart;
            quint16 colFact;
            quint16 colNorm;
            quint16 colResult;
            quint16 rowStrokeTime;
        };

        explicit TechnicalResults(Layout layout) : m_layout(std::move(layout)) {}

        void build(Writer& writer, const Context& ctx) override;

    private:
        QString resultOk(CrossingStatus::State state) const;
        QString resultLimit(CrossingStatus::State state) const;

        Layout m_layout;
    };
}