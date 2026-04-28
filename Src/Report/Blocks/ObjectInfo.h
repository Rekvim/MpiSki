#pragma once

#include "IBlock.h"

namespace Report::Blocks {
    class ObjectInfo : public IBlock {
    public:
        struct Layout {
            QString sheet;
            int rowStart;
            int column;
        };

        explicit ObjectInfo(Layout layout) : m_layout(std::move(layout)) {}

        void build(Writer& writer, const Context& ctx) override
        {
            writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.object);
            writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.manufactory);
            writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.department);
        }
    private:
        Layout m_layout;
    };
}
