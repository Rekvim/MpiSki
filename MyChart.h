#ifndef MYCHART_H
#define MYCHART_H

#include <QObject>
#include <QtCharts>

class MySeries : public QLineSeries
{
    Q_OBJECT
private:
    quint8 axisN;

public:
    explicit MySeries(QObject *parent = nullptr, quint8 axN = 0);
    quint8 get_axisN() const;
};

class MyChart : public QChartView
{
    Q_OBJECT
private:
    QString name;
    const qreal minR = 0.1;
    const qreal minR_Time = 1000;
    qreal minRange;
    qreal maxRange = 0;
    QList<QValueAxis *> Yaxis;
    QValueAxis *Xaxis_Value;
    QDateTimeAxis *Xaxis_Time;
    QAbstractAxis *Xaxis;
    QLineSeries markerX, markerY;

    QList<MySeries *> Series;
    QList<MySeries *> Series_dubl;
    qreal minX, maxX;
    qreal axisX_min, axisX_max;
    bool empty;
    bool zoomed;
    bool update;

    qreal X1, X2;
    QGraphicsSimpleTextItem *m_coordItem = NULL;
    QPoint markersPos;

    void drawMarkers(QPoint pos);

    void ZoomIn(qreal, qreal);
    void ZoomOut();

    void autoscale(qreal min, qreal max);

public:
    MyChart(QWidget *parent = nullptr);

    ~MyChart();
    QString getname() const;
    void setname(QString name);
    void setMaxRange(qreal value);
    QPair<QList<QPointF>, QList<QPointF>> getpoints(quint8 seriesN) const;
    void savetostream(QDataStream &stream) const;
    void loadfromstream(QDataStream &stream);

public slots:
    void useTimeaxis(bool);
    void addAxis(QString);
    void addSeries(quint8 axisN, QString name, QColor color);
    void addPoint(quint8 seriesN, qreal X, qreal Y);
    void dublSeries(quint8 seriesN);
    void clear();
    void visible(quint8 seriesN, bool visible);
    void showdots(bool show);
    void setLabelXformat(QString);
    void autoupdate(bool);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // MYCHART_H
