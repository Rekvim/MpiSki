#ifndef OPTIONTEST_H
#define OPTIONTEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QDateTime>
#include "Test.h"

class OptionTest : public Test
{
    Q_OBJECT

public:
    explicit OptionTest(QObject *parent = nullptr, bool end_test_after_process = true);
    virtual void Process() override;

    struct Task
    {
        QVector<quint16> value;
        quint32 delay;
    };

    void SetTask(Task task);

private:
    Task task_;
    const bool end_test_after_process_;
signals:
    void SetStartTime();
};

#endif // OPTIONTEST_H
