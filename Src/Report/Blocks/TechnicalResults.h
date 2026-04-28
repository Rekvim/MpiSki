#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class TechnicalResults : public IBlock {
    public:
        struct Layout {
            QString sheet;
            int rowStart;
            int colFact;
            int colNorm;
            int colResult;
            int rowStrokeTime;
        };

        explicit TechnicalResults(Layout layout) : m_layout(std::move(layout)) {}

        void build(Writer& writer, const Context& ctx) override;

    private:
        QString resultOk(CrossingStatus::State state) const;
        QString resultLimit(CrossingStatus::State state) const;

        Layout m_layout;
    };
}