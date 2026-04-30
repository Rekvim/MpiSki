#pragma once

#include <QString>

namespace Report {

class Writer;
struct Context;

namespace Blocks {
class ValveSpec final {
public:
    struct Layout {
        QString sheet;
        int rowStart;
        int column;
        bool positionerModel = false;
        bool includeSolenoid = false;
    };

    explicit ValveSpec(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};
}
}