#pragma once

#include "Storage/Telemetry.h"

namespace Ui {
class MainWindow;
}

class TelemetryUiMapper
{
public:
    TelemetryUiMapper(Ui::MainWindow* ui) : m_ui(ui) { }

    void updateInit(const InitState& init);

    void updateMainTest(const Domain::Tests::Main::Result& mainResult,
                        const ValveStrokeRecord& strokeResult);

    void updateStrokeTest(const Domain::Tests::Stroke::Result& result);

    void updateStepTest(const Domain::Tests::Option::Step::Result& result);

    void updateCyclicRegulatoryTest(
        const Domain::Tests::Cyclic::Regulatory::Result& result);

    void updateCyclicShutoffTest(
        const Domain::Tests::Cyclic::Shutoff::Result& result);

    void updateCrossingValues(const Domain::Tests::Main::Result& mainResult,
                              const ValveStrokeRecord& strokeResult);

private:
    Ui::MainWindow* m_ui;
};