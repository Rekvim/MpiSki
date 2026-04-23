#pragma once

#include "Domain/Tests/Test.h"

namespace Domain::Tests::Option {
    class Algorithm : public Test
    {
        Q_OBJECT

    public:
        explicit Algorithm(QObject *parent = nullptr)
            : Test(parent) {}

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