#include "ImageService.h"
#include "Manager.h"
#include "ChartView.h"
#include "Report/Saver.h"

namespace Widgets::Chart {
QImage ImageService::captureChart(ChartType chart)
{
    auto backup = hideAuxSeries(chart);

    ChartView* ch = m_charts->chart(chart);
    if (!ch)
        return {};

    QPixmap pix = ch->grab();

    if (backup)
        restoreSeries(chart, *backup);

    return pix.toImage();
}

void ImageService::restoreSeries(ChartType chart, const SeriesVisibilityBackup& b)
{
    auto* ch = m_charts->chart(chart);
    if (!ch) return;

    if (chart == ChartType::Task && b.visible.size() == 3) {
        ch->visible(2, b.visible[0]);
        ch->visible(3, b.visible[1]);
        ch->visible(4, b.visible[2]);
    }

    if (chart == ChartType::Pressure && b.visible.size() == 1) {
        ch->visible(1, b.visible[0]);
    }
}

std::optional<ImageService::SeriesVisibilityBackup>
ImageService::hideAuxSeries(ChartType chart)
{
    auto* ch = m_charts->chart(chart);
    if (!ch) return std::nullopt;

    SeriesVisibilityBackup b;

    if (chart == ChartType::Task) {

        b.visible = {
            ch->series()[2]->isVisible(),
            ch->series()[3]->isVisible(),
            ch->series()[4]->isVisible()
        };

        ch->visible(2,false);
        ch->visible(3,false);
        ch->visible(4,false);
    }

    if (chart == ChartType::Pressure) {
        b.visible = {ch->series()[1]->isVisible()};
        ch->visible(1,false);
    }

    return b;
}

void ImageService::saveChart(ChartType chart, QLabel* preview, QImage& target)
{
    ChartView* ch = m_charts->chart(chart);
    if (!ch)
        return;

    if (m_saver)
        m_saver->saveImage(ch);

    QImage img = captureChart(chart);

    target = img;

    if (preview)
        preview->setPixmap(QPixmap::fromImage(img));
}

void ImageService::saveChart(ChartType chart)
{
    ChartView* ch = m_charts->chart(chart);
    if (!ch)
        return;

    if (m_saver)
        m_saver->saveImage(ch);
}
}