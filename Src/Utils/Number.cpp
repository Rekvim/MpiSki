#include "Number.h"
#include <QLocale>

qreal
NumberUtils::toDouble(QString s, bool* okOut)
{
    s = s.trimmed();
    s.replace(',', '.');

    bool ok = false;
    const qreal v = QLocale::c().toDouble(s, &ok);

    if (okOut)
        *okOut = ok;

    return v;
}

std::optional<QPair<double,double>>
NumberUtils::parseRange(QString s)
{
    s = s.trimmed();

    s.replace(QChar(0x2013), '-');
    s.replace(QChar(0x2014), '-');
    s.replace(QChar(0x2212), '-');

    const QStringList parts = s.split('-', Qt::SkipEmptyParts);
    if (parts.size() != 2)
        return std::nullopt;

    bool ok1 = false, ok2 = false;
    const double a = toDouble(parts[0], &ok1);
    const double b = toDouble(parts[1], &ok2);

    if (!ok1 || !ok2)
        return std::nullopt;

    const double low = qMin(a, b);
    const double high = qMax(a, b);

    return QPair<double,double>(low, high);
}