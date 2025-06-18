#ifndef MYSERIES_H
#define MYSERIES_H

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
};

#endif // MYSERIES_H
