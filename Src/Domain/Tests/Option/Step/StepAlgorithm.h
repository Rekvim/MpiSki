#pragma once

#include "Src/Domain/Tests/Option/OptionAlgorithm.h"
#include "StepResult.h"

#include <QPointF>

namespace Domain::Tests::Option::Step {
    class Algorithm : public Option::Algorithm
    {
        Q_OBJECT
    public:
        explicit Algorithm(QObject *parent = nullptr)
            : Option::Algorithm(parent) {}

        void run() override;
        void setTValue(quint32 T_value);

    private:
        QVector<Result> calculateResult(const QVector<QVector<QPointF>> &points) const;
        quint32 m_TValue;
    signals:
        void getPoints(QVector<QVector<QPointF>> &points);
        void results(QVector<Result> result, quint32 T_value);
    };
}