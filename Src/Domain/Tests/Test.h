#pragma once

#include <QEventLoop>
#include <QObject>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr);
    virtual ~Test() = default;

protected:
    QEventLoop *m_eventLoop;
    bool m_terminate = false;

    void Sleep(quint16 msecs);
    void setDacBlocked(quint16 value,
                       quint32 sleepMs = 0,
                       bool waitForStop = false,
                       bool waitForStart = false);

public slots:
    virtual void run() = 0;
    void requestStop();
    void releaseWait();

signals:
    void dacCommandRequested(quint16 value,
                quint32 sleepMs = 0,
                bool waitForStop = false,
                bool waitForStart = false);
    void UpdateGraph();
    void finished();
    void executionStarted();
};
