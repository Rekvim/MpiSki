#include "TechnicalResults.h"

#include "Gui/Setup/ValveWindow/ValveEnums.h"

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

    void TechnicalResults::build(Writer& writer, const Context& ctx)
    {
        const auto& t = ctx.telemetry;
        const auto& v = ctx.valve;

        const quint16 dynamicError = m_layout.rowStart;
        const quint16 stroke = m_layout.rowStart + 2;
        const quint16 spring = m_layout.rowStart + 4;
        const quint16 pressure = m_layout.rowStart + 6;
        const quint16 frictionPercent = m_layout.rowStart + 8;
        const quint16 frictionForce = m_layout.rowStart + 10;

        if (v.dinamicErrorRecomend == "Без позиционера") {
            writer.cell(m_layout.sheet, dynamicError, m_layout.colResult, v.dinamicErrorRecomend);
        } else {
            writer.cell(m_layout.sheet, dynamicError, m_layout.colFact, f2(t.mainTestRecord.dynamicErrorReal));
            writer.cell(m_layout.sheet, dynamicError, m_layout.colNorm, v.dinamicErrorRecomend);
            writer.cell(m_layout.sheet, dynamicError, m_layout.colResult, resultOk(t.crossingStatus.dynamicError));
        }

        writer.cell(m_layout.sheet, stroke, m_layout.colFact, f2(t.valveStrokeRecord.real));
        writer.cell(m_layout.sheet, stroke, m_layout.colNorm, v.valveStroke);
        writer.cell(m_layout.sheet, stroke, m_layout.colResult, resultOk(t.crossingStatus.valveStroke));

        const bool driveDD2 = (v.driveType == DriveType::DoubleActing);
        if (driveDD2) {
            writer.cell(m_layout.sheet, spring, m_layout.colResult, "Привод ДД");
        } else {
            writer.cell(m_layout.sheet, spring, m_layout.colFact,
                   QString("%1–%2").arg(f2(t.mainTestRecord.springLow),
                                        f2(t.mainTestRecord.springHigh)));
            writer.cell(m_layout.sheet, spring, m_layout.colNorm,
                   QString("%1–%2").arg(f2(v.driveRangeLow),
                                        f2(v.driveRangeHigh)));
            writer.cell(m_layout.sheet, spring, m_layout.colResult, resultOk(t.crossingStatus.spring));
        }

        writer.cell(m_layout.sheet, pressure, m_layout.colFact,
               QString("%1–%2").arg(f2(t.mainTestRecord.lowLimitPressure),
                                    f2(t.mainTestRecord.highLimitPressure)));

        writer.cell(m_layout.sheet, frictionPercent, m_layout.colFact, f2(t.mainTestRecord.frictionPercent));
        writer.cell(m_layout.sheet, frictionPercent, m_layout.colResult, resultLimit(t.crossingStatus.frictionPercent));

        writer.cell(m_layout.sheet, frictionForce, m_layout.colFact, f3(t.mainTestRecord.frictionForce));

        // stroke times
        // if (t.stroke) {
        //     writer.cell(m_layout.sheet, m_layout.rowStrokeTime, 5, t.stroke->timeForwardMs);
        //     writer.cell(m_layout.sheet, m_layout.rowStrokeTime, 8, t.stroke->timeBackwardMs);
        // }
    }
}