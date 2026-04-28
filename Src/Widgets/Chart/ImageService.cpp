#include "ImageService.h"

#include <QOpenGLWidget>
#include <QPainter>

#include "Manager.h"
#include "ChartView.h"

namespace Widgets::Chart {

ImageService::ImageService(Manager* charts, QObject* parent)
    : QObject(parent)
    , m_charts(charts)
{
}

QImage ImageService::captureChart(ChartType chart)
{
    auto backup = hideAuxSeries(chart);

    ChartView* ch = m_charts->chart(chart);
    if (!ch)
        return {};

    QPixmap pixmap = ch->grab();

    if (auto* glWidget = ch->findChild<QOpenGLWidget*>()) {
        QPainter painter(&pixmap);
        const QPoint delta =
            glWidget->mapToGlobal(QPoint()) - ch->mapToGlobal(QPoint());
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawImage(delta, glWidget->grabFramebuffer());
    }

    if (backup)
        restoreSeries(chart, *backup);

    return pixmap.toImage();
}

void ImageService::restoreSeries(ChartType chart, const SeriesVisibilityBackup& b)
{
    auto* ch = m_charts ? m_charts->chart(chart) : nullptr;

    if (!ch)
        return;

    if (chart == ChartType::Task && b.visible.size() == 3) {
        ch->visible(2, b.visible[0]);
        ch->visible(3, b.visible[1]);
        ch->visible(4, b.visible[2]);
        return;
    }

    if (chart == ChartType::Pressure && b.visible.size() == 1) {
        ch->visible(1, b.visible[0]);
        return;
    }
}

std::optional<ImageService::SeriesVisibilityBackup>
ImageService::hideAuxSeries(ChartType chart)
{
    auto* ch = m_charts ? m_charts->chart(chart) : nullptr;

    if (!ch)
        return std::nullopt;

    SeriesVisibilityBackup b;

    if (chart == ChartType::Task) {
        if (ch->series().size() <= 4)
            return std::nullopt;

        b.visible = {
            ch->series()[2]->isVisible(),
            ch->series()[3]->isVisible(),
            ch->series()[4]->isVisible()
        };

        ch->visible(2, false);
        ch->visible(3, false);
        ch->visible(4, false);

        return b;
    }

    if (chart == ChartType::Pressure) {
        if (ch->series().size() <= 1)
            return std::nullopt;

        b.visible = {
            ch->series()[1]->isVisible()
        };

        ch->visible(1, false);

        return b;
    }

    return std::nullopt;
}

}