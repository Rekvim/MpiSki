#include "ChartImageService.h"
#include "ChartManager.h"
#include "MyChart.h"
#include "Src/Report/Saver.h"

ChartImageService::ChartImageService(
    ChartManager* charts,
    Report::Saver* saver)
    : m_charts(charts),
    m_saver(saver)
{
}

QImage ChartImageService::captureChart(Charts chart)
{
    auto backup = hideAuxSeries(chart);

    MyChart* ch = m_charts->chart(chart);
    if (!ch)
        return {};

    QPixmap pix = ch->grab();

    if (backup)
        restoreSeries(chart, *backup);

    return pix.toImage();
}

void ChartImageService::restoreSeries(Charts chart, const SeriesVisibilityBackup& b)
{
    auto* ch = m_charts->chart(chart);
    if (!ch) return;

    if (chart == Charts::Task && b.visible.size() == 3) {
        ch->visible(2, b.visible[0]);
        ch->visible(3, b.visible[1]);
        ch->visible(4, b.visible[2]);
    }

    if (chart == Charts::Pressure && b.visible.size() == 1) {
        ch->visible(1, b.visible[0]);
    }
}

std::optional<ChartImageService::SeriesVisibilityBackup>
ChartImageService::hideAuxSeries(Charts chart)
{
    auto* ch = m_charts->chart(chart);
    if (!ch)
        return std::nullopt;

    SeriesVisibilityBackup b;

    if (chart == Charts::Task) {

        b.visible = {
            ch->series()[2]->isVisible(),
            ch->series()[3]->isVisible(),
            ch->series()[4]->isVisible()
        };

        ch->visible(2,false);
        ch->visible(3,false);
        ch->visible(4,false);
    }

    if (chart == Charts::Pressure) {

        b.visible = {
            ch->series()[1]->isVisible()
    };

    ch->visible(1,false);
}

return b;
}

void ChartImageService::saveChart(
    Charts chart,
    QLabel* preview,
    QImage& target)
{
    MyChart* ch = m_charts->chart(chart);
    if (!ch)
        return;

    if (m_saver)
        m_saver->saveImage(ch);

    QImage img = captureChart(chart);

    target = img;

    if (preview)
        preview->setPixmap(QPixmap::fromImage(img));
}

void ChartImageService::saveChart(Charts chart)
{
    MyChart* ch = m_charts->chart(chart);
    if (!ch)
        return;

    if (m_saver)
        m_saver->saveImage(ch);
}
