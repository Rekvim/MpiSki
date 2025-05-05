#ifndef TEST_H
#define TEST_H

#pragma once
#include <QEventLoop>
#include <QObject>
#include <QPointF>
#include <QTimer>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr);
    virtual ~Test() = default;

protected:
    QTimer *m_graphTimer;
    QEventLoop *m_eventLoop;
    bool m_terminate;

    void Sleep(quint16 msecs);
    void SetDACBlocked(quint16 value,
                       quint32 sleepMs = 0,
                       bool waitForStop = false,
                       bool waitForStart = false);

public slots:
    virtual void Process() = 0;
    void Stop();
    void ReleaseBlock();

signals:
    void SetDAC(quint16 value,
                quint32 sleepMs = 0,
                bool waitForStop = false,
                bool waitForStart = false);
    void UpdateGraph();
    void EndTest();
};
#endif // TEST_H
