#pragma once

#include <QString>

namespace Report {
class Writer;
struct Context;
namespace Blocks {

class PositionSensorDeviationRanges final {
public:
    struct Layout {
        QString sheet;
        int rowStart;
        int rangeColumn;
        int rowStep;
        int forwardValueColumn;
        int forwardCycleColumn;
        int backwardValueColumn;
        int backwardCycleColumn;
    };

    explicit PositionSensorDeviationRanges(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};
}
}