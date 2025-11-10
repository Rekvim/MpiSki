#ifndef ITESTRUNNER_H
#define ITESTRUNNER_H

#pragma once
#include <QObject>
#include <QPoint>

#include "./Src/Telemetry/TelemetryStore.h"

class TelemetryStore;

class ITestRunner : public QObject {
    Q_OBJECT
public:
    explicit ITestRunner(QObject* parent=nullptr) : QObject(parent) {}
    ~ITestRunner() override = default;

public slots:
    virtual void start() = 0;  // запустить тест
    virtual void stop() = 0;  // попросить тест завершиться

signals:
    void totalTestTimeMs(quint64);
    void endTest();

    // Унифицированные события для UI
    void addPoints(int chart, const QVector<struct Point>&);
    void clearPoints(int chart);

    void telemetryUpdated(const TelemetryStore&); // если понадобится
};


#endif // ITESTRUNNER_H
