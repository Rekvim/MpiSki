#pragma once
#include <QString>

namespace NumberUtils
{
qreal toDouble(QString s, bool* okOut = nullptr);

std::optional<QPair<double,double>>
parseRange(QString s);
}