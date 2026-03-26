#pragma once

#include "ui_MainWindow.h"
#include "Src/Storage/Telemetry.h"

class TelemetryUiMapper
{
public:
    explicit TelemetryUiMapper(Ui::MainWindow* ui);

    void updateInit(const InitState& init);
    void updateMainTest(const TelemetryStore& t);
    void updateStrokeTest(const StrokeTestRecord& r);
    void updateCrossing(const TelemetryStore& t);

private:
    Ui::MainWindow* m_ui;

    void updateCrossingIndicators(const TelemetryStore& t);
};
