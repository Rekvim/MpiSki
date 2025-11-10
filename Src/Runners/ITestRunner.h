#ifndef ITESTRUNNER_H
#define ITESTRUNNER_H

#pragma once
#include <QObject>
#include <QPoint>

class TelemetryStore;

class ITestRunner : public QObject {
    Q_OBJECT
public:
    explicit ITestRunner(QObject* parent=nullptr) : QObject(parent) {}
    ~ITestRunner() override = default;

public slots:
    virtual void start() = 0;  // запустить тест
    virtual void stop() = 0;  // попросить тест завершиться
    virtual void releaseBlock() = 0;   // форвард внутрь воркера

signals:
    void requestClearChart(int chartIndex);
    void totalTestTimeMs(quint64);
    void endTest();

    // Унифицированные события для UI
    void addPoints(int chart, const QVector<struct QPoint>&);
    void clearPoints(int chart);

    void telemetryUpdated(const TelemetryStore&); // если понадобится
    void requestSetDAC(quint16 dac, quint32 sleepMs, bool waitForStop, bool waitForStart);

};


#endif // ITESTRUNNER_H
