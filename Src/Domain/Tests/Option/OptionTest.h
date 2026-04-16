#pragma once

#include "Src/Domain/Tests/Test.h"

class OptionTest : public Test
{
    Q_OBJECT

public:
    explicit OptionTest(QObject *parent = nullptr);
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
