#pragma once

#include "Domain/Tests/AbstractTestAlgorithm.h"

namespace Domain::Tests::Option {
    class Algorithm : public AbstractTestAlgorithm
    {
        Q_OBJECT

    public:
        explicit Algorithm(QObject *parent = nullptr)
            : AbstractTestAlgorithm(parent) {}

        virtual void run() override;

        struct Task
        {
            QVector<quint16> value;
            quint32 delay;
        };

        void setTask(Task task);

    private:
        Task m_task;
    };
}