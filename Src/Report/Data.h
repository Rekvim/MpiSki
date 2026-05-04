#pragma once

#include <QString>
#include <QVector>
#include <QImage>

namespace Report {

struct CellData {
    QString sheet;
    quint16 row = 0;
    quint16 col = 0;
    QString value;
};

struct ValidationData {
    QString formula;
    QString range;
};

struct ImageCell {
    QString sheet;
    int row = 0;
    int col = 0;
    QImage image;
};

struct ReportData {
    QVector<CellData> data;
    QVector<ValidationData> validation;
    QVector<ImageCell> images;
};

}