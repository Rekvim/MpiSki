#pragma once

#include "Report/Writer.h"
#include "Report/Builder.h"

namespace Report::Blocks {
    class IBlock {
    public:
        virtual ~IBlock() = default;

        virtual void build(Writer& writer, const Context& ctx) = 0;
    };
}