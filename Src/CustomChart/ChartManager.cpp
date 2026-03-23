#include "ChartManager.h"

ChartManager::ChartManager(QObject* parent)
    : QObject(parent)
{
}

void ChartManager::registerChart(Charts type, MyChart* chart)
{
    m_charts[type] = chart;
}

MyChart* ChartManager::chart(Charts type)
{
    return m_charts.value(type, nullptr);
}

void ChartManager::addPoints(Charts chart, const QVector<Point> &points)
{
    if (!m_charts.contains(chart))
        return;

    for (const auto& p : points)
        m_charts[chart]->addPoint(p.seriesNum, p.X, p.Y);
}

void ChartManager::clearPoints(Charts chart)
{
    if (!m_charts.contains(chart))
        return;

    m_charts[chart]->clear();
}

void ChartManager::setVisible(Charts chart, quint16 series, bool visible)
{
    if (!m_charts.contains(chart))
        return;

    m_charts[chart]->visible(series, visible);
}

void ChartManager::showDots(bool visible)
{
    m_charts[Charts::Task]->showDots(visible);

    if (m_charts.contains(Charts::Pressure))
        m_charts[Charts::Pressure]->showDots(visible);
}

void ChartManager::duplicateMainChartsSeries()
{
    if (!m_charts.contains(Charts::Task))
        return;

    auto* task = m_charts[Charts::Task];

    task->duplicateChartSeries(1);
    task->duplicateChartSeries(2);
    task->duplicateChartSeries(3);
    task->duplicateChartSeries(4);

    if (m_charts.contains(Charts::Pressure))
        m_charts[Charts::Pressure]->duplicateChartSeries(0);
}

QPair<QList<QPointF>, QList<QPointF>>
ChartManager::getPoints(Charts chart, int series)
{
    if (!m_charts.contains(chart))
        return {};

    return m_charts[chart]->getPoints(series);
}

QPixmap ChartManager::grabChart(Charts chart)
{
    if (!m_charts.contains(chart))
        return {};

    return m_charts[chart]->grab();
}

MyChart* ChartManager::createTrendChart(MyChart* chart, const QColor& linearColor)
{
    m_charts[Charts::Trend] = chart;

    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);
    chart->setMaxRange(60000);

    return chart;
}

MyChart* ChartManager::createStrokeChart(MyChart* chart, const QColor& linearColor)
{
    m_charts[Charts::Stroke] = chart;

    chart->setName(QStringLiteral("Stroke"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}


MyChart* ChartManager::createTaskChart(
    MyChart* chart,
    const QString& strokeAxisFormat,
    const QColor& linearColor,
    const QColor& pressure1Color,
    const QColor& pressure2Color,
    const QColor& pressure3Color
    ) {
    m_charts[Charts::Task] = chart;

    chart->setName(QStringLiteral("Task"));
    chart->useTimeaxis(false);
    chart->addAxis(QStringLiteral("%.2f bar"));
    chart->addAxis(strokeAxisFormat);
    chart->addSeries(1, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(1, QObject::tr("Датчик линейных перемещений"), linearColor);
    chart->addSeries(0, QObject::tr("Датчик давления 1"), pressure1Color);
    chart->addSeries(0, QObject::tr("Датчик давления 2"), pressure2Color);
    chart->addSeries(0, QObject::tr("Датчик давления 3"), pressure3Color);

    return chart;
}

MyChart* ChartManager::createFrictionChart(
    MyChart* chart,
    const QString& strokeAxisFormat,
    const QColor& linearColor
    ) {
    m_charts[Charts::Friction] = chart;

    chart->setName(QStringLiteral("Friction"));
    chart->addAxis(QStringLiteral("%.2f N"));
    chart->addSeries(0, QObject::tr("Трение от перемещения"), linearColor);
    chart->setLabelXformat(strokeAxisFormat);

    return chart;
}

MyChart* ChartManager::createPressureChart(
    MyChart* chart,
    const QString& strokeAxisFormat,
    const QColor& linearColor
    ) {
    m_charts[Charts::Pressure] = chart;

    chart->setName(QStringLiteral("Pressure"));
    chart->useTimeaxis(false);
    chart->setLabelXformat(QStringLiteral("%.2f bar"));
    chart->addAxis(strokeAxisFormat);
    chart->addSeries(0, QObject::tr("Перемещение от давления"), linearColor);
    chart->addSeries(0, QObject::tr("Линейная регрессия"), QColor::fromRgb(0, 0, 0));
    chart->visible(1, false);

    return chart;
}

MyChart* ChartManager::createResponseChart(
    MyChart* chart,
    const QColor& linearColor
    ) {
    m_charts[Charts::Response] = chart;

    chart->setName(QStringLiteral("Response"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}

MyChart* ChartManager::createResolutionChart(
    MyChart* chart,
    const QColor& linearColor
    ) {
    m_charts[Charts::Resolution] = chart;

    chart->setName(QStringLiteral("Resolution"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor::fromRgb(0, 0, 0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}

MyChart* ChartManager::createStepChart(MyChart* chart, const QColor& linearColor)
{
    m_charts[Charts::Step] = chart;

    chart->setName(QStringLiteral("Step"));
    chart->useTimeaxis(true);
    chart->addAxis(QStringLiteral("%.2f%%"));
    chart->addSeries(0, QObject::tr("Задание"), QColor(0,0,0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    return chart;
}

MyChart* ChartManager::createCyclicChart(MyChart* chart,
                                         const QColor& linearColor)
{
    m_charts[Charts::Cyclic] = chart;

    chart->setName("Cyclic");
    chart->useTimeaxis(true);
    chart->addAxis("%.2f%%");

    chart->addSeries(0, QObject::tr("Задание"), QColor(0,0,0));
    chart->addSeries(0, QObject::tr("Датчик линейных перемещений"), linearColor);

    chart->setMaxRange(240000);

    return chart;
}