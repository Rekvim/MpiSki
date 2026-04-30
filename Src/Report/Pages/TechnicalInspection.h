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

        int positionRow;
        int positionColumn = 9;

        int objectInfoRow;
        int objectInfoColumn = 4;

        int valveSpecRow;
        int valveSpecColumn = 13;

        int technicalResultsRow;
        int factColumn = 5;
        int normColumn = 8;
        int resultColumn = 11;
        int strokeTimeRow;

        int firstDateRow;
        int firstDateColumn = 12;

        int fioRow;
        int fioColumn = 4;

        int taskImageRow;
        int taskImageColumn = 1;

        int pressureImageRow;
        int pressureImageColumn = 1;

        int frictionImageRow;
        int frictionImageColumn = 1;

        int secondDateRow;
        int secondDateColumn = 12;

        bool positionerModel = true;
        bool includeSolenoid = false;
    };

    explicit TechnicalInspection(Layout layout);

    void build(Writer& writer, const Context& ctx);

private:
    Layout m_layout;
};

}
}