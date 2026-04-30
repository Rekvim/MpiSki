#pragma once

#include "Report/Blocks/CyclicSummary.h"

#include <QString>

namespace Report {

class Writer;
struct Context;

namespace Pages {

class CyclicDeviation final {
public:
    struct Layout {
        QString sheet;

        int positionRow = 0;
        int positionColumn = 9;

        int objectInfoRow = 0;
        int objectInfoColumn = 4;

        int valveSpecRow = 0;
        int valveSpecColumn = 13;

        int summaryRow = 0;
        int summaryColumn = 8;
        int summaryRowStep = 2;

        int deviationRangesRow = 0;
        int deviationRangeColumn = 2;
        int deviationRowStep = 2;
        int deviationForwardValueColumn = 8;
        int deviationForwardCycleColumn = 11;
        int deviationBackwardValueColumn = 12;
        int deviationBackwardCycleColumn = 15;

        int fioRow = 0;
        int fioColumn = 4;

        int dateRow = 0;
        int dateColumn = 12;

        Blocks::CyclicSummary::CyclicMode mode =
            Blocks::CyclicSummary::CyclicMode::Regulatory;

        bool positionerModel = true;
        bool includeSolenoid = false;
    };

    explicit CyclicDeviation(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};

}

}