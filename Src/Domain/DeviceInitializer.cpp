#include "DeviceInitializer.h"
#include <QtMath>

DeviceInitializer::DeviceInitializer(Mpi& mpi,
                                     Registry& registry,
                                     TelemetryStore& telemetry)
    : m_mpi(mpi), m_registry(registry), m_telemetry(telemetry)
{}

// Подключение и инициализация устройства
bool DeviceInitializer::connectAndInitDevice()
{
    bool ok = m_mpi.isConnect();

    m_telemetry.init.deviceStatusText =
        ok ? QString("Успешное подключение к порту %1").arg(m_mpi.portName())
           : "Ошибка подключения";

    m_telemetry.init.deviceStatusColor =
        ok ? Qt::darkGreen : Qt::red;

    if (!ok) return false;

    ok = m_mpi.initialize();

    m_telemetry.init.initStatusText =
        ok ? "Успешная инициализация" : "Ошибка инициализации";

    m_telemetry.init.initStatusColor =
        ok ? Qt::darkGreen : Qt::red;

    return ok;
}
// Обнаружение и отчёт по датчикам
bool DeviceInitializer::detectSensors()
{
    int cnt = m_mpi.sensorCount();

    if (cnt == 0) {
        m_telemetry.init.connectedSensorsText = "Датчики не обнаружены";
        m_telemetry.init.connectedSensorsColor = Qt::red;
    }
    else if (cnt == 1) {
        m_telemetry.init.connectedSensorsText = "Обнаружен 1 датчик";
        m_telemetry.init.connectedSensorsColor = Qt::darkYellow;
    }
    else {
        m_telemetry.init.connectedSensorsText =
            QString("Обнаружено %1 датчика").arg(cnt);

        m_telemetry.init.connectedSensorsColor = Qt::darkGreen;
    }

    return cnt > 0;
}

void DeviceInitializer::measureStartPosition(bool normalClosed)
{
    m_telemetry.init.startingPositionText = "Измерение";
    m_telemetry.init.startingPositionColor = Qt::darkYellow;

    if (normalClosed)
        m_mpi[0]->captureMin();
    else
        m_mpi[0]->captureMax();

    m_telemetry.init.startingPositionText = m_mpi[0]->formattedValue();
    m_telemetry.init.startingPositionColor = Qt::darkGreen;
}

void DeviceInitializer::measureEndPosition(bool normalClosed)
{
    m_telemetry.init.finalPositionText = "Измерение";
    m_telemetry.init.finalPositionColor = Qt::darkYellow;

    if (normalClosed)
        m_mpi[0]->captureMax();
    else
        m_mpi[0]->captureMin();

    m_telemetry.init.finalPositionText = m_mpi[0]->formattedValue();
    m_telemetry.init.finalPositionColor = Qt::darkGreen;
}

void DeviceInitializer::measureStartPositionShutoff(
    bool normalClosed,
    QVector<bool>& initialStates,
    const QVector<bool>& savedStates)
{
    m_telemetry.init.startingPositionText = "Измерение";
    m_telemetry.init.startingPositionColor = Qt::darkYellow;

    for (int i = 0; i < savedStates.size(); ++i) {
        if (savedStates[i]) {
            initialStates[i] = false;
            m_mpi.setDiscreteOutput(i, false);
        }
    }

    if (normalClosed)
        m_mpi[0]->captureMin();
    else
        m_mpi[0]->captureMax();

    m_telemetry.init.startingPositionText = m_mpi[0]->formattedValue();
    m_telemetry.init.startingPositionColor = Qt::darkGreen;
}

void DeviceInitializer::measureEndPositionShutoff(
    bool normalClosed,
    QVector<bool>& initialStates,
    const QVector<bool>& savedStates)
{
    m_telemetry.init.finalPositionText = "Измерение";
    m_telemetry.init.finalPositionColor = Qt::darkYellow;

    for (int i = 0; i < savedStates.size(); ++i) {
        if (savedStates[i]) {
            initialStates[i] = true;
            m_mpi.setDiscreteOutput(i, true);
        }
    }

    if (normalClosed) m_mpi[0]->captureMax();
    else m_mpi[0]->captureMin();

    m_telemetry.init.finalPositionText = m_mpi[0]->formattedValue();
    m_telemetry.init.finalPositionColor = Qt::darkGreen;
}

void DeviceInitializer::calculateCoefficients()
{
    const auto& valveInfo = m_registry.valveInfo();

    qreal coeff = 1.0;

    if (valveInfo.strokeMovement == StrokeMovement::Rotary) {
        coeff = qRadiansToDegrees(2.0 / valveInfo.diameterPulley);
        m_mpi[0]->setUnit("°");
    }

    m_mpi[0]->correctCoefficients(coeff);
}

void DeviceInitializer::recordStrokeRange(bool normalClosed)
{
    if (normalClosed) {
        m_telemetry.valveStrokeRecord.range = m_mpi[0]->formattedValue();
        m_telemetry.valveStrokeRecord.real = m_mpi[0]->value();
    } else {
        m_telemetry.valveStrokeRecord.range = m_mpi[0]->formattedValue();
        m_telemetry.valveStrokeRecord.real = m_mpi[0]->value();
    }
}
