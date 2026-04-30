#pragma once

#include <QString>

namespace Report {

class Writer;
struct Context;

namespace Blocks {

class ObjectInfo final {
public:
    struct Layout {
        QString sheet;
        int rowStart;
        int column;
    };

    explicit ObjectInfo(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};

}

}