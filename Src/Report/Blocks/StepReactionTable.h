#pragma once

#include <QString>

namespace Report {

class Writer;
struct Context;

namespace Blocks {
class StepReactionTable final {
public:
    struct Layout {
        QString sheet;

        int imageRow;
        int imageCol;

        int startRow;
        int firstBaseCol;
        int secondBaseCol;
    };

    explicit StepReactionTable(Layout layout);

    void build(Writer& writer, const Context& ctx);
private:
    Layout m_layout;
};
}
}