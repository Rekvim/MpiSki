#pragma once

#include "IBlock.h"
#include "./Src/Ui/Setup/ValveWindow/ValveEnums.h"

namespace Report::Blocks {
    class ValveSpec : public IBlock {
    public:
        struct Layout {
            QString sheet;
            quint16 rowStart;
            quint16 column;
            bool positionerModel = false;
            bool includeSolenoid = false;
        };
        explicit ValveSpec(Layout layout)
            : m_layout(layout) {}

        void build(Writer& writer,
                   const Context& ctx) override
        {
            quint16 row = m_layout.rowStart;
            const auto& valve = ctx.valve;
            const auto& telemetry = ctx.telemetry;
            const auto& params = ctx.params;

            writer.cell(m_layout.sheet, row++, m_layout.column, valve.positionNumber);
            writer.cell(m_layout.sheet, row++, m_layout.column, valve.serialNumber);
            writer.cell(m_layout.sheet, row++, m_layout.column, valve.valveModel);
            writer.cell(m_layout.sheet, row++, m_layout.column, valve.manufacturer);
            writer.cell(m_layout.sheet, row++, m_layout.column, QString("%1 / %2").arg(valve.DN, valve.PN));

            if (m_layout.positionerModel)
                writer.cell(m_layout.sheet, row++, m_layout.column, valve.positionerModel);

            if (m_layout.includeSolenoid) {
                    writer.cell(m_layout.sheet, row++, m_layout.column, valve.solenoidValveModel);

                writer.cell(m_layout.sheet, row++, m_layout.column,
                            QString("%1 / %2").arg(valve.limitSwitchModel, valve.positionSensorModel));
            }
            writer.cell(m_layout.sheet, row++, m_layout.column, QString::number(telemetry.supplyRecord.pressure_bar, 'f', 2));
            writer.cell(m_layout.sheet, row++, m_layout.column, params.safePosition);
            writer.cell(m_layout.sheet, row++, m_layout.column, valve.driveModel);
            writer.cell(m_layout.sheet, row++, m_layout.column, params.strokeMovement);
            writer.cell(m_layout.sheet, row++, m_layout.column,
                            ValveEnums::StuffingBoxSealToString(valve.materialStuffingBoxSeal));
        }

    private:
        Layout m_layout;
    };
}