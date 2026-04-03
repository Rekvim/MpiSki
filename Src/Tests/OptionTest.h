#pragma once

#include "Test.h"

class OptionTest : public Test
{
    Q_OBJECT

public:
    explicit OptionTest(QObject *parent = nullptr);
    virtual void Process() override;

    struct Task
    {
        QVector<quint16> value;
        quint32 delay;
    };

    void SetTask(Task task);

private:
    Task m_task;
};
