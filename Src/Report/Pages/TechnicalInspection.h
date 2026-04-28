#pragma once

#include <QString>

namespace Report {
class Writer;
struct Context;

namespace Pages {
class TechnicalInspection final {
public:
    struct Layout {
        QString sheet;
        quint16 rowStart;
        bool includeSolenoid = false;
    };

    explicit TechnicalInspection(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};
}
}