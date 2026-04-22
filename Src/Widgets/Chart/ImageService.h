#pragma once

#include <QImage>
#include <optional>
#include "Src/Report/Saver.h"
#include "ChartView.h"
#include "Manager.h"

namespace Widgets::Chart {
class ImageService
{
public:
    struct SeriesVisibilityBackup
    {
        QVector<bool> visible;
    };

    ImageService(Manager* charts, Report::Saver* saver)
        : m_charts(charts), m_saver(saver) {}

    QImage captureChart(ChartType chart);

    void saveChart(ChartType chart, QLabel* previewLabel, QImage& targetImage);
    void saveChart(ChartType chart);

private:
    std::optional<SeriesVisibilityBackup> hideAuxSeries(ChartType chart);
    void restoreSeries(ChartType chart, const SeriesVisibilityBackup& backup);

    Manager* m_charts;
    Report::Saver* m_saver;
};
}