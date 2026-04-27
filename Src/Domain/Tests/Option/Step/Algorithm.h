#pragma once

#include "Domain/Tests/Option/Algorithm.h"
#include "Result.h"

#include <QPointF>

namespace Domain::Tests::Option::Step {
    class Algorithm : public Option::Algorithm
    {
        Q_OBJECT
    public:
        explicit Algorithm(QObject *parent = nullptr) : Option::Algorithm(parent) {}

        void run() override;
        void setTValue(quint32 T_value);

    signals:
        void getPoints(QVector<QVector<QPointF>> &points);
        void results(const Result& result);

    private:
        Result calculateResult(const QVector<QVector<QPointF>>& points) const;
        quint32 m_TValue;
    };
}