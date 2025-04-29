#include "MyChart.h"

MySeries::MySeries(QObject *parent, quint8 axN)
    : QLineSeries{parent}
{
    axisN = axN;
}

quint8 MySeries::get_axisN() const
{
    return axisN;
}

MyChart::MyChart(QWidget *parent)
    : QChartView(parent)
{
    update = true;

    Xaxis_Time = new QDateTimeAxis(this);
    Xaxis_Value = new QValueAxis(this);

    //    QFont font_time = Xaxis_Time->labelsFont();
    //    font_time.setPixelSize(13);
    //    Xaxis_Time->setLabelsFont(font_time);

    //    QFont font_value = Xaxis_Time->labelsFont();
    //    font_value.setPixelSize(13);
    //    Xaxis_Time->setLabelsFont(font_value);

    Xaxis = Xaxis_Value;
    minRange = minR;

    Xaxis_Value->setLabelFormat("%.2f mA");
    Xaxis_Value->setRange(0, minR);
    Xaxis_Value->setMinorTickCount(4);

    Xaxis_Time->setFormat("mm:ss.zzz");
    Xaxis_Time->setRange(QDateTime::fromMSecsSinceEpoch(0),
                         QDateTime::fromMSecsSinceEpoch(minR_Time));

    chart()->addAxis(Xaxis, Qt::AlignBottom);

    empty = true;
    zoomed = false;

    chart()->addSeries(&markerX);
    chart()->addSeries(&markerY);

    markerX.attachAxis(Xaxis);
    markerY.attachAxis(Xaxis);

    markerX.setColor(Qt::gray);
    markerY.setColor(Qt::gray);

    chart()->legend()->markers(&markerX)[0]->setVisible(false);
    chart()->legend()->markers(&markerY)[0]->setVisible(false);

    markerX.setUseOpenGL();
    markerY.setUseOpenGL();

    m_coordItem = new QGraphicsSimpleTextItem(this->chart());
    QFont F;
    F.setPointSize(10);
    m_coordItem->setFont(F);
    m_coordItem->show();
    m_coordItem->setVisible(false);

    QOpenGLWidget *glWidget = this->findChild<QOpenGLWidget *>();
    glWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
}

MyChart::~MyChart() {}

void MyChart::drawMarkers(QPoint pos)
{
    QSet<quint8> Set;
    QList<MySeries *> Ser;

    Set.clear();

    for (MySeries *Serial : Series) {
        if (!Set.contains(Serial->get_axisN())) {
            if (Serial->isVisible()) {
                Set.insert(Serial->get_axisN());
                Ser.push_back(Serial);
            }
        }
    }

    QPointF curVal = this->chart()->mapToValue(pos);

    QString format;
    QString coordStr = "";

    for (MySeries *Serial : Ser) {
        format = Yaxis.at(Serial->get_axisN())->labelFormat();
        coordStr += QString::asprintf(format.toLocal8Bit(),
                                      this->chart()->mapToValue(pos, Serial).y())
                    + "\n";
    }

    if (Xaxis == Xaxis_Value) {
        format = Xaxis_Value->labelFormat();
        coordStr += QString::asprintf(format.toLocal8Bit(), curVal.x());
    } else {
        QTime time = QTime::fromMSecsSinceStartOfDay(curVal.x());
        coordStr += time.toString(Xaxis_Time->format());
    }

    m_coordItem->setText(coordStr);
    m_coordItem->setPos(pos.x() + 10, pos.y() - 20 * (Set.count() + 1));

    markerX.clear();

    qreal min, max;

    if (Xaxis == Xaxis_Value) {
        min = Xaxis_Value->min();
        max = Xaxis_Value->max();
    } else {
        min = Xaxis_Time->min().toMSecsSinceEpoch();
        max = Xaxis_Time->max().toMSecsSinceEpoch();
    }

    markerX.append(min, curVal.y());
    markerX.append(max, curVal.y());
    markerX.setVisible(true);

    markerY.clear();
    markerY.append(curVal.x(), Yaxis.last()->min());
    markerY.append(curVal.x(), Yaxis.last()->max());
    markerY.setVisible(true);

    chart()->legend()->markers(&markerX)[0]->setVisible(false);
    chart()->legend()->markers(&markerY)[0]->setVisible(false);
}

void MyChart::setLabelXformat(QString format)
{
    Xaxis_Value->setLabelFormat(format);
}

void MyChart::autoupdate(bool u)
{
    update = u;

    if (u) {
        axisX_min = minX;
        axisX_max = qMax(maxX, minX + minRange);

        if (Xaxis == Xaxis_Value) {
            axisX_min = qMax(qFloor(axisX_min), 0);
            axisX_max = qCeil(axisX_max);
            Xaxis->setRange(axisX_min, axisX_max);
        } else {
            axisX_min = qMax(qFloor(axisX_min / 1000.0), 0) * 1000.0;
            qMax(axisX_min + 1000.0, axisX_max = qCeil(axisX_max / 1000.0) * 1000.0);
            Xaxis->setRange(QDateTime::fromMSecsSinceEpoch(qCeil(axisX_min)),
                            QDateTime::fromMSecsSinceEpoch(qCeil(axisX_max)));
        }

        autoscale(minX, maxX);
    }
}

void MyChart::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->setRubberBand(QChartView::HorizontalRubberBand);
        QPointF curVal = this->chart()->mapToValue(event->pos());
        X1 = curVal.x();
    }

    QChartView::mousePressEvent(event);
}

void MyChart::mouseMoveEvent(QMouseEvent *event)
{
    if (Series.count()) {
        m_coordItem->setVisible(true);
        markersPos = event->pos();

        drawMarkers(markersPos);
    }

    QChartView::mouseMoveEvent(event);
}

void MyChart::mouseReleaseEvent(QMouseEvent *event)
{
    if (Series.count()) {
        if (event->button() == Qt::LeftButton) {
            QPointF curVal = this->chart()->mapToValue(event->pos());
            X2 = curVal.x();

            ZoomIn(qMin(X1, X2), qMax(X1, X2));
        }

        if (event->button() == Qt::RightButton) {
            ZoomOut();
        }
    }

    this->setRubberBand(QChartView::NoRubberBand);
    QChartView::mouseReleaseEvent(event);
}

void MyChart::leaveEvent(QEvent *)
{
    m_coordItem->setVisible(false);
    markerX.setVisible(false);
    markerY.setVisible(false);
}

void MyChart::useTimeaxis(bool useTime)
{
    markerX.detachAxis(Xaxis);
    markerY.detachAxis(Xaxis);

    chart()->removeAxis(Xaxis);

    if (useTime) {
        Xaxis = Xaxis_Time;
        minRange = minR_Time;
    } else {
        Xaxis = Xaxis_Value;
        minRange = minR;
    }

    chart()->addAxis(Xaxis, Qt::AlignBottom);

    markerX.attachAxis(Xaxis);
    markerY.attachAxis(Xaxis);
}

void MyChart::addAxis(QString format)
{
    if (Yaxis.count()) {
        markerX.detachAxis(Yaxis.last());
        markerY.detachAxis(Yaxis.last());
    }

    Yaxis.emplace_back(new QValueAxis(this));
    Yaxis.last()->setLabelFormat(format);
    chart()->addAxis(Yaxis.last(), Qt::AlignLeft);

    Yaxis.last()->setRange(0, 0.01);
    Yaxis.last()->setMinorTickCount(4);

    //    QFont font = Yaxis.last()->labelsFont();
    //    font.setPixelSize(13);
    //    Yaxis.last()->setLabelsFont(font);

    markerX.attachAxis(Yaxis.last());
    markerY.attachAxis(Yaxis.last());
}

void MyChart::addSeries(quint8 axisN, QString name, QColor color)
{
    if (axisN >= Yaxis.count()) {
        return;
    }

    Series_dubl.emplace_back(new MySeries(this, axisN));

    chart()->addSeries(Series_dubl.last());

    Series_dubl.last()->setColor(color.lighter(170));

    Series_dubl.last()->attachAxis(Yaxis[axisN]);
    Series_dubl.last()->attachAxis(Xaxis);

    chart()->legend()->markers(Series_dubl.last())[0]->setVisible(false);

    this->thread()->msleep(50);

    Series.push_back(new MySeries(this, axisN));

    chart()->addSeries(Series.last());

    Series.last()->setName(name);
    Series.last()->setColor(color);

    Series.last()->attachAxis(Yaxis[axisN]);
    Series.last()->attachAxis(Xaxis);
}

void MyChart::addPoint(quint8 seriesN, qreal X, qreal Y)
{
    if (seriesN >= Series.count()) {
        return;
    }

    if (empty) {
        minX = X;
        maxX = X;
        empty = false;
    } else {
        minX = qMin(minX, X);
        maxX = qMax(maxX, X);
    }

    if (maxRange != 0) {
        minX = qMax(minX, maxX - maxRange);
    }

    if ((!zoomed) and (update)) {
        axisX_min = minX;
        axisX_max = qMax(maxX, minX + minRange);

        if (Xaxis == Xaxis_Value) {
            axisX_min = qMax(qFloor(axisX_min), 0);
            axisX_max = qCeil(axisX_max);
            Xaxis->setRange(axisX_min, axisX_max);
        } else {
            axisX_min = qMax(qFloor(axisX_min / 1000.0), 0) * 1000.0;
            axisX_max = qMax(axisX_min + 1000.0, qCeil(axisX_max / 1000.0) * 1000.0);
            Xaxis->setRange(QDateTime::fromMSecsSinceEpoch(qCeil(axisX_min)),
                            QDateTime::fromMSecsSinceEpoch(qCeil(axisX_max)));
        }

        autoscale(axisX_min, axisX_max);
    }

    Series[seriesN]->append(X, Y);
}

void MyChart::dublSeries(quint8 seriesN)
{
    if (seriesN >= Series.count()) {
        return;
    }

    if (Series[seriesN]->points().empty()) {
        return;
    }

    Series_dubl[seriesN]->replace(Series.at(seriesN)->points());

    Series[seriesN]->clear();
}

void MyChart::clear()
{
    for (MySeries *S : Series) {
        S->clear();
    }

    for (MySeries *S : Series_dubl) {
        S->clear();
    }

    empty = true;
    zoomed = false;

    Xaxis_Value->setRange(0, minR);
    Xaxis_Time->setRange(QDateTime::fromMSecsSinceEpoch(0),
                         QDateTime::fromMSecsSinceEpoch(minR_Time));
}

void MyChart::visible(quint8 seriesN, bool visible)
{
    bool showaxis = visible;

    if (seriesN >= Series.count()) {
        return;
    }

    Series[seriesN]->setVisible(visible);

    Series_dubl[seriesN]->setVisible(visible);

    chart()->legend()->markers(Series_dubl[seriesN])[0]->setVisible(false);

    if (!visible)
        for (int i = 0; i < Series.count(); i++) {
            if (i == seriesN) {
                continue;
            }

            if (Series.at(i)->get_axisN() == Series.at(seriesN)->get_axisN()) {
                if (Series.at(i)->isVisible()) {
                    showaxis = true;
                    break;
                }
            }
        }

    Yaxis[Series.at(seriesN)->get_axisN()]->setVisible(showaxis);

    autoscale(axisX_min, axisX_max);
}

void MyChart::showdots(bool show)
{
    for (MySeries *S : Series) {
        S->setPointsVisible(show);
    }

    for (MySeries *S : Series_dubl) {
        S->setPointsVisible(show);
    }
}

void MyChart::autoscale(qreal min, qreal max)
{
    for (int ax = 0; ax < Yaxis.count(); ax++) {
        bool first = true;
        qreal Ymin = 0;
        qreal Ymax = 0;

        foreach (const MySeries *S, Series) {
            if ((S->get_axisN() == ax) and (S->isVisible())) {
                foreach (QPointF value, S->points()) {
                    if ((value.x() >= min) and (value.x() <= max)) {
                        if (first) {
                            first = false;
                            Ymin = value.y();
                            Ymax = value.y();
                        } else {
                            Ymin = qMin(value.y(), Ymin);
                            Ymax = qMax(value.y(), Ymax);
                        }
                    }
                }
            }
        }

        foreach (const MySeries *S, Series_dubl) {
            if ((S->get_axisN() == ax) and (S->isVisible())) {
                foreach (QPointF value, S->points()) {
                    if ((value.x() >= min) and (value.x() <= max)) {
                        if (first) {
                            first = false;
                            Ymin = value.y();
                            Ymax = value.y();
                        } else {
                            Ymin = qMin(value.y(), Ymin);
                            Ymax = qMax(value.y(), Ymax);
                        }
                    }
                }
            }
        }

        qreal Llim = 0;

        qreal Hlim = qMax(1.0, Ymax * 1.1);
        Hlim = qCeil(Hlim);

        Yaxis[ax]->setRange(Llim, Hlim);
    }

    if (m_coordItem)
        if (m_coordItem->isVisible()) {
            drawMarkers(markersPos);
        }
}

void MyChart::ZoomIn(qreal min, qreal max)
{
    min = qMax(min, minX);
    max = qMin(max, maxX);

    if ((max - min) <= minRange) {
        min -= (minRange - max + min) / 2;
        max = min + minRange;
    }

    if (max > maxX) {
        min -= (max - maxX);
        max = maxX;
    }

    if (min < minX) {
        max += (minX - min);
        min = minX;
    }

    if (Xaxis == Xaxis_Value) {
        Xaxis->setRange(min, max);
    } else {
        Xaxis->setRange(QDateTime::fromMSecsSinceEpoch(min),
                        QDateTime::fromMSecsSinceEpoch(qCeil(max)));
    }

    axisX_min = min;
    axisX_max = max;

    zoomed = ((axisX_max < maxX) or (axisX_min > minX));

    autoscale(min, max);
}

void MyChart::ZoomOut()
{
    if (!zoomed) {
        return;
    }

    qreal min = axisX_min - (axisX_max - axisX_min) / 2;
    qreal max = axisX_max + (axisX_max - axisX_min) / 2;

    if ((max - min) > (maxX - minX)) {
        zoomed = false;
        min = minX;
        max = minX + qMax(minRange, maxX - minX);
    }

    if (max > maxX) {
        min -= (max - maxX);
        max = maxX;
    }

    if (min < minX) {
        max += (minX - min);
        min = minX;
    }

    if (Xaxis == Xaxis_Value) {
        min = qMax(qFloor(min), 0);
        max = qCeil(max);
        Xaxis->setRange(min, max);
    } else {
        min = qMax(qFloor(min / 1000.0), 0) * 1000.0;
        max = qMax(min + 1000.0, qCeil(max / 1000.0) * 1000.0);
        Xaxis->setRange(QDateTime::fromMSecsSinceEpoch(qCeil(min)),
                        QDateTime::fromMSecsSinceEpoch(qCeil(max)));
    }

    axisX_min = min;
    axisX_max = max;

    autoscale(min, max);
}

void MyChart::setname(QString n)
{
    name = n;
}

void MyChart::setMaxRange(qreal value)
{
    maxRange = value;
}

QString MyChart::getname() const
{
    return name;
}

QPair<QList<QPointF>, QList<QPointF>> MyChart::getpoints(quint8 seriesN) const
{
    return qMakePair(Series.at(seriesN)->points(), Series_dubl.at(seriesN)->points());
}

void MyChart::savetostream(QDataStream &stream) const
{
    stream << name;
    stream << (Xaxis == Xaxis_Value);
    stream << Xaxis_Value->labelFormat();

    stream << Yaxis.size();

    for (const QValueAxis *y : Yaxis) {
        stream << y->labelFormat();
    }

    stream << Series.size();

    for (const auto serial : Series) {
        stream << serial->get_axisN();
        stream << serial->name();
        stream << serial->color();
    }

    for (const auto serial : Series) {
        stream << serial->points();
    }

    for (const auto serial : Series_dubl) {
        stream << serial->points();
    }

    stream << Series.at(0)->pointsVisible();
}

void MyChart::loadfromstream(QDataStream &stream)
{
    QString str;
    stream >> str;
    setname(str);

    bool value_axis;
    stream >> value_axis;
    useTimeaxis(!value_axis);

    stream >> str;
    setLabelXformat(str);

    qsizetype Y_size;
    stream >> Y_size;

    for (quint32 i = 0; i < Y_size; ++i) {
        stream >> str;
        addAxis(str);
    }

    qsizetype S_size;
    stream >> S_size;

    for (quint32 i = 0; i < S_size; ++i) {
        quint8 axisN;
        QString name;
        QColor color;
        stream >> axisN >> name >> color;
        addSeries(axisN, name, color);
    }

    for (const auto serial : Series) {
        QList<QPointF> points;
        stream >> points;
        serial->append(points);
    }

    for (const auto serial : Series_dubl) {
        QList<QPointF> points;
        stream >> points;
        serial->append(points);
    }

    autoupdate(true);
}
