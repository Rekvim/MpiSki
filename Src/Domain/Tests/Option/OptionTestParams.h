#pragma once

#include <QVector>
#include <QMetaType>

struct OptionTestParams
{
    quint32 delay;
    QVector<qreal> points;
    QVector<qreal> steps;
};
Q_DECLARE_METATYPE(QMetaType)

