#pragma once

#include <QImage>
#include <optional>
#include "Src/Report/Saver.h"

class QLabel;
class ChartManager;
class MyChart;

enum class Charts;

class ChartImageService
{
public:
    struct SeriesVisibilityBackup
    {
        QVector<bool> visible;
    };

    ChartImageService(ChartManager* charts,
                      Report::Saver* saver);

    QImage captureChart(Charts chart);

    void saveChart(Charts chart, QLabel* previewLabel, QImage& targetImage);
    void saveChart(Charts chart);

private:
    std::optional<SeriesVisibilityBackup> hideAuxSeries(Charts chart);
    void restoreSeries(Charts chart, const SeriesVisibilityBackup& backup);

    ChartManager* m_charts;
    Report::Saver* m_saver;
};
