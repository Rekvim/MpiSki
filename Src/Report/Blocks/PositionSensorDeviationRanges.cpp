#include "PositionSensorDeviationRanges.h"

namespace Report::Blocks {
PositionSensorDeviationRanges::PositionSensorDeviationRanges(Layout layout)
    : m_layout(std::move(layout))
{}

void PositionSensorDeviationRanges::build(Writer& writer, const Context& ctx)
{
    Q_UNUSED(writer)
    Q_UNUSED(ctx)

    // TODO:
    // Заполнить отклонение от заданного значения сигнала
    // датчика положения позиционера.
    //
    // По структуре блок аналогичен CyclicRegulatoryRanges,
    // но данные должны браться из другого источника telemetry.
}
}