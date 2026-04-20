#pragma once

#include "Src/Report/Writer.h"
#include "Src/Report/Builder.h"
#include "IBlock.h"

namespace Report::Blocks {
    class TechnicalResults : public IBlock {
    public:
        struct Layout {
            QString sheet;
            quint16 rowStart; // 22
            quint16 colFact; // 5
            quint16 colNorm; // 8
            quint16 colResult; // 11
            quint16 rowStrokeTime; // 48
        };

        explicit TechnicalResults(Layout l) : m(l) {}

        void build(Writer& w, const Context& ctx) override;

    private:
        QString resultOk(CrossingStatus::State state) const;
        QString resultLimit(CrossingStatus::State state) const;

        Layout m;
    };
}