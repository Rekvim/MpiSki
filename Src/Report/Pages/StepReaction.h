#pragma once

#include <QString>

namespace Report {
class Writer;
struct Context;
namespace Pages {

class StepReaction final {
public:
    struct Layout {
        QString sheet;
        int rowStart;
        bool includeSolenoid = false;
    };

    explicit StepReaction(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};
}
}