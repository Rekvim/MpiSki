#pragma once

#include <QObject>
#include <QLineSeries>

class MySeries : public QLineSeries
{
    Q_OBJECT
private:
    quint8 m_axisN;

public:
    explicit MySeries(QObject *parent = nullptr, quint8 axN = 0);
    quint8 getAxisN() const;

    void setMarkersOnly(bool on);
    bool isMarkersOnly() const;

private:
    bool m_markersOnly = false;
};
