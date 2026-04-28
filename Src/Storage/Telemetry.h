#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include "Domain/Tests/Stroke/Result.h"
#include "Domain/Tests/Main/Result.h"
#include "Domain/Tests/Option/Step/Result.h"
#include "Domain/Tests/Cyclic/Regulatory/Result.h"
#include "Domain/Tests/Cyclic/Shutoff/Result.h"

struct InitState {
    QString deviceStatusText = "";
    QColor deviceStatusColor = QColor();
    QString initStatusText = "";
    QColor initStatusColor = QColor();
    QString connectedSensorsText = "";
    QColor connectedSensorsColor = QColor();
    QString startingPositionText = "";
    QColor startingPositionColor = QColor();
    QString finalPositionText = "";
    QColor finalPositionColor = QColor();
};

struct CrossingStatus {
    enum class State {
        Unknown,
        Ok,
        Fail
    };

    State frictionPercent = State::Unknown;
    State valveStroke = State::Unknown;
    State dynamicError = State::Unknown;
    State spring = State::Unknown;
    State linearCharacteristic = State::Unknown;
};

struct ValveStrokeRecord {
    QString range = "";
    qreal real= 0.0;
};

struct SupplyRecord {
    double pressure_bar = 0.0;
};

class Telemetry {
public:
    InitState init;
    ValveStrokeRecord valveStrokeRecord;
    SupplyRecord supplyRecord;
    CrossingStatus crossingStatus;

    std::optional<Domain::Tests::Stroke::Result> testStroke;
    std::optional<Domain::Tests::Main::Result> testMain;
    std::optional<Domain::Tests::Option::Step::Result> testStep;

    std::optional<Domain::Tests::Cyclic::Regulatory::Result> testСyclicRegulatory;
    std::optional<Domain::Tests::Cyclic::Shutoff::Result> testСyclicShutoff;

    Telemetry() = default;

    void clearAll() {
        init = {};
        testMain.reset();
        testStroke.reset();
        testStep.reset();
        testСyclicRegulatory.reset();
        testСyclicShutoff.reset();
        valveStrokeRecord = {};
        supplyRecord = {};
    }
};
