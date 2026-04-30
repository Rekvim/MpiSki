#pragma once

#include <QString>

namespace Report {

class Writer;
struct Context;

namespace Blocks {
    class SolenoidDetails final {
    public:
        struct Layout {
            QString sheet;

            int rowBase;
            int colCount;
            int colOn;
            int colOff;
            int rowStep;

            int rowSwitch1;
            int rowSwitch2;
        };

        explicit SolenoidDetails(Layout layout);

        void build(Writer& w, const Context& ctx);

    private:
        Layout m_layout;
    };
}
}