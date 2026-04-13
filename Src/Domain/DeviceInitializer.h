#pragma once

#include "Src/Domain/Mpi/Mpi.h"
#include "Src/Storage/Registry.h"
#include "Src/Storage/Telemetry.h"

class DeviceInitializer
{
public:
    DeviceInitializer(
        Mpi& mpi,
        Registry& registry,
        Telemetry& telemetry);

    bool connectAndInitDevice();
    bool detectSensors();

    void measureStartPosition(bool normalClosed);
    void measureEndPosition(bool normalClosed);

    void measureStartPositionShutoff(bool normalClosed,
                                     QVector<bool>& initialStates,
                                     const QVector<bool>& savedStates);

    void measureEndPositionShutoff(bool normalClosed,
                                   QVector<bool>& initialStates,
                                   const QVector<bool>& savedStates);

    void calculateCoefficients();
    void recordStrokeRange(bool normalClosed);

private:
    Mpi& m_mpi;
    Registry& m_registry;
    Telemetry& m_telemetry;
};
