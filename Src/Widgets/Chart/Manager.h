#pragma once

#include <QHash>
#include <QVector>
#include <QPointF>

#include "ChartView.h"
#include "ChartType.h"
#include "Point.h"

namespace Widgets::Chart {
class Manager : public QObject
{
    Q_OBJECT

public:
    explicit Manager(QObject* parent = nullptr) : QObject(parent) {}

    struct SeriesVisibilityBackup
    {
        QVector<bool> visible;
    };

    void duplicateMainChartsSeries();

    QPair<QList<QPointF>, QList<QPointF>> getPoints(Widgets::Chart::ChartType chart, int series);

    QPixmap grabChart(Widgets::Chart::ChartType chart);

    SeriesVisibilityBackup hidePressureAuxSeries();
    SeriesVisibilityBackup hideTaskAuxSeries();

    void restoreSeries(Widgets::Chart::ChartType chart, const SeriesVisibilityBackup& backup);

    ChartView* chart(Widgets::Chart::ChartType type);
    ChartView* createTrendChart(ChartView* chart, const QColor& linearColor);
    ChartView* createStrokeChart(ChartView* chart, const QColor& linearColor);
    ChartView* createTaskChart(
        ChartView* chart,
        const QString& strokeAxisFormat,
        const QColor& linearColor,
        const QColor& pressure1Color,
        const QColor& pressure2Color,
        const QColor& pressure3Color
    );
    ChartView* createFrictionChart(
        ChartView* chart,
        const QString& strokeAxisFormat,
        const QColor& linearColor
    );
    ChartView* createPressureChart(
        ChartView* chart,
        const QString& strokeAxisFormat,
        const QColor& linearColor
    );
    ChartView* createResponseChart(
        ChartView* chart,
        const QColor& linearColo
        );
    ChartView* createResolutionChart(
        ChartView* chart,
        const QColor& linearColo
    );
    ChartView* createStepChart(ChartView* chart, const QColor& linearColor);
    ChartView* createCyclicChart(ChartView* chart, const QColor& linearColor);
public slots:
    void addPoints(Widgets::Chart::ChartType chart, const QVector<Widgets::Chart::Point> &points);
    void clearPoints(Widgets::Chart::ChartType chart);
    void setVisible(Widgets::Chart::ChartType chart, quint16 series, bool visible);
    void showDots(bool visible);

private:
    QHash<Widgets::Chart::ChartType, ChartView*> m_charts;
};
}