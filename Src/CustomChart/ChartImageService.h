#pragma once

#include <QImage>
#include <optional>

class QLabel;
class ChartManager;
class ReportSaver;
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
                      ReportSaver* saver);

    QImage captureChart(Charts chart);

    void saveChart(Charts chart, QLabel* previewLabel, QImage& targetImage);
    void saveChart(Charts chart);

private:
    std::optional<SeriesVisibilityBackup> hideAuxSeries(Charts chart);
    void restoreSeries(Charts chart, const SeriesVisibilityBackup& backup);

    ChartManager* m_charts;
    ReportSaver* m_saver;
};
