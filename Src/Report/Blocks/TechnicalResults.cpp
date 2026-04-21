#include "TechnicalResults.h"

#include "./Src/Gui/Setup/ValveWindow/ValveEnums.h"

namespace Report::Blocks {
    static QString f2(qreal v) { return QString::number(v, 'f', 2); }
    static QString f3(qreal v) { return QString::number(v, 'f', 3); }

    QString TechnicalResults::resultOk(CrossingStatus::State state) const {
        using State = CrossingStatus::State;
        switch (state) {
            case State::Ok: return "соответствует";
            case State::Fail: return "не соответствует";
            case State::Unknown: return "не определено";
        }
        return {};
    }

    QString TechnicalResults::resultLimit(CrossingStatus::State state) const {
        using State = CrossingStatus::State;
        switch (state) {
            case State::Ok: return "не превышает";
            case State::Fail: return "превышает";
            case State::Unknown: return "не определено";
        }
        return {};
    }

    void TechnicalResults::build(Writer& w, const Context& ctx)
    {
        const auto& t = ctx.telemetry;
        const auto& v = ctx.valve;

        const quint16 dynamicError = m.rowStart;
        const quint16 stroke = m.rowStart + 2;
        const quint16 spring = m.rowStart + 4;
        const quint16 pressure = m.rowStart + 6;
        const quint16 frictionPercent = m.rowStart + 8;
        const quint16 frictionForce = m.rowStart + 10;

        if (v.dinamicErrorRecomend == "Без позиционера") {
            w.cell(m.sheet, dynamicError, m.colResult, v.dinamicErrorRecomend);
        } else {
            w.cell(m.sheet, dynamicError, m.colFact, f2(t.mainTestRecord.dynamicErrorReal));
            w.cell(m.sheet, dynamicError, m.colNorm, v.dinamicErrorRecomend);
            w.cell(m.sheet, dynamicError, m.colResult, resultOk(t.crossingStatus.dynamicError));
        }

        w.cell(m.sheet, stroke, m.colFact, f2(t.valveStrokeRecord.real));
        w.cell(m.sheet, stroke, m.colNorm, v.valveStroke);
        w.cell(m.sheet, stroke, m.colResult, resultOk(t.crossingStatus.valveStroke));

        const bool driveDD2 = (v.driveType == DriveType::DoubleActing);
        if (driveDD2) {
            w.cell(m.sheet, spring, m.colResult, "Привод ДД");
        } else {
            w.cell(m.sheet, spring, m.colFact,
                   QString("%1–%2").arg(f2(t.mainTestRecord.springLow),
                                        f2(t.mainTestRecord.springHigh)));
            w.cell(m.sheet, spring, m.colNorm,
                   QString("%1–%2").arg(f2(v.driveRangeLow),
                                        f2(v.driveRangeHigh)));
            w.cell(m.sheet, spring, m.colResult, resultOk(t.crossingStatus.spring));
        }

        w.cell(m.sheet, pressure, m.colFact,
               QString("%1–%2").arg(f2(t.mainTestRecord.lowLimitPressure),
                                    f2(t.mainTestRecord.highLimitPressure)));

        w.cell(m.sheet, frictionPercent, m.colFact, f2(t.mainTestRecord.frictionPercent));
        w.cell(m.sheet, frictionPercent, m.colResult, resultLimit(t.crossingStatus.frictionPercent));

        w.cell(m.sheet, frictionForce, m.colFact, f3(t.mainTestRecord.frictionForce));

        // stroke times
        // if (t.stroke) {
        //     w.cell(m.sheet, m.rowStrokeTime, 5, t.stroke->timeForwardMs);
        //     w.cell(m.sheet, m.rowStrokeTime, 8, t.stroke->timeBackwardMs);
        // }
    }
}