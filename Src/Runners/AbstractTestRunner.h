#pragma once

#include <QObject>
#include <QPoint>

class TelemetryStore;

class AbstractTestRunner : public QObject {
    Q_OBJECT
public:
    explicit AbstractTestRunner(QObject* parent=nullptr) : QObject(parent) {}
    ~AbstractTestRunner() override = default;

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void releaseBlock() = 0;

signals:
    void requestClearChart(int chartIndex);
    void totalTestTimeMs(quint64);
    void endTest();
    void testActuallyStarted();
    void requestSetDAC(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart);
};
