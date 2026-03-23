#pragma once

#include <QHash>
#include <QVector>
#include <QPointF>

#include "MyChart.h"

enum class Charts
{
    Task,
    Pressure,
    Friction,
    Response,
    Resolution,
    Stroke,
    Step,
    Trend,
    Cyclic
};

struct Point
{
    quint8 seriesNum;
    qreal X;
    qreal Y;
};

class ChartManager : public QObject
{
    Q_OBJECT

public:
    explicit ChartManager(QObject* parent = nullptr);

    struct SeriesVisibilityBackup
    {
        QVector<bool> visible;
    };

    void registerChart(Charts type, MyChart* chart);

    void clearPoints(Charts chart);

    void setVisible(Charts chart, quint16 series, bool visible);

    void showDots(bool visible);

    void duplicateMainChartsSeries();

    QPair<QList<QPointF>, QList<QPointF>> getPoints(Charts chart, int series);

    QPixmap grabChart(Charts chart);

    SeriesVisibilityBackup hidePressureAuxSeries();
    SeriesVisibilityBackup hideTaskAuxSeries();

    void restoreSeries(Charts chart, const SeriesVisibilityBackup& backup);

    MyChart* chart(Charts type);
    MyChart* createTrendChart(MyChart* chart, const QColor& linearColor);
    MyChart* createStrokeChart(MyChart* chart, const QColor& linearColor);
    MyChart* createTaskChart(
        MyChart* chart,
        const QString& strokeAxisFormat,
        const QColor& linearColor,
        const QColor& pressure1Color,
        const QColor& pressure2Color,
        const QColor& pressure3Color
    );
    MyChart* createFrictionChart(
        MyChart* chart,
        const QString& strokeAxisFormat,
        const QColor& linearColor
    );
    MyChart* createPressureChart(
        MyChart* chart,
        const QString& strokeAxisFormat,
        const QColor& linearColor
    );
    MyChart* createResponseChart(
        MyChart* chart,
        const QColor& linearColo
        );
    MyChart* createResolutionChart(
        MyChart* chart,
        const QColor& linearColo
    );
    MyChart* createStepChart(MyChart* chart, const QColor& linearColor);
    MyChart* createCyclicChart(MyChart* chart, const QColor& linearColor);
public slots:
    void addPoints(Charts chart, const QVector<Point> &points);

private:
    QHash<Charts, MyChart*> m_charts;
};