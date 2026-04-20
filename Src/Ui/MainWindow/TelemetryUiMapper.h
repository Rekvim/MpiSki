#pragma once

#include "ui_MainWindow.h"
#include "Src/Storage/Telemetry.h"
#include "Src/Domain/Tests/Stroke/StrokeResult.h"

class TelemetryUiMapper
{
public:
    TelemetryUiMapper(Ui::MainWindow* ui)
        : m_ui(ui) { }

    void updateInit(const InitState& init);
    void updateMainTest(const Telemetry& t);
    void updateStrokeTest(const Domain::Tests::Stroke::Result& r);
    void updateCrossing(const Telemetry& t);

private:
    Ui::MainWindow* m_ui;

    void updateCrossingIndicators(const Telemetry& t);
};
