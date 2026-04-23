#pragma once

#include "Storage/Telemetry.h"
#include "Domain/Tests/Stroke/Result.h"

namespace Ui {
class MainWindow;
}

class TelemetryUiMapper
{
public:
    TelemetryUiMapper(Ui::MainWindow* ui) : m_ui(ui) { }

    void updateInit(const InitState& init);
    void updateMainTest(const Telemetry& t);
    void updateStrokeTest(const Domain::Tests::Stroke::Result& r);
    void updateCrossing(const Telemetry& t);

private:
    Ui::MainWindow* m_ui;

    void updateCrossingIndicators(const Telemetry& t);
};