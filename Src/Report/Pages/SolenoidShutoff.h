#pragma once

#include <QString>

namespace Report {
class Writer;
struct Context;

namespace Pages {
class SolenoidShutoff final {
public:
    struct Layout {
        QString sheet;
        int rowStart;
    };

    explicit SolenoidShutoff(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};
}
}