#ifndef REPORTSAVER_H
#define REPORTSAVER_H

#pragma once
#include <QDir>
#include <QObject>
#include <QImage>
#include "./Src/CustomChart/MyChart.h"
#include "Registry.h"

struct ImageCell {
    QString sheet;
    int row;
    int col;
    QImage image;
};

class ReportSaver : public QObject
{
    Q_OBJECT
public:

    struct ReportData {
        QString sheet;
        quint16 x;
        quint16 y;
        QString value;
    };

    struct ValueBinding {
        int row, col;
        QString value;
    };

    struct ValidationData {
        QString formula;
        QString range;
    };

    struct Report {
        QVector<ReportData> data;
        QVector<ValidationData> validation;
        QVector<ImageCell> images;
    };

    explicit ReportSaver(QObject *parent = nullptr);
    void SetRegistry(Registry *registry);
    void SaveImage(MyChart *chart);
    QDir Directory();
    bool SaveReport(const Report &report, const QString &templatePath);
private:
    void CreateDir();
    QDir m_dir;
    bool m_created;
    Registry *m_registry;
signals:
    void Question(QString title, QString text, bool &result);
    void GetDirectory(QString current_path, QString &result);
};

#endif // REPORTSAVER_H
