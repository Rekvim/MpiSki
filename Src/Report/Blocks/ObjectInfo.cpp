#include "ObjectInfo.h"

#include "Report/Writer.h"
#include "Report/Builder.h"

namespace Report::Blocks {

ObjectInfo::ObjectInfo(Layout layout)
    : m_layout(std::move(layout))
{}

void ObjectInfo::build(Writer& writer, const Context& ctx)
{
    writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.object);
    writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.manufactory);
    writer.cell(m_layout.sheet, m_layout.rowStart++, m_layout.column, ctx.object.department);
}
}