#pragma once

#include <QObject>
#include <QDir>
#include <QImage>

#include "Widgets/Chart/ChartView.h"
#include "Widgets/Chart/ChartType.h"

namespace Report {
class Saver : public QObject
{
    Q_OBJECT

public:
    struct Data {
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

    struct Report {
        QVector<Data> data;
        QVector<ValidationData> validation;
        QVector<ImageCell> images;
    };

    explicit Saver(QObject* parent = nullptr);
    ~Saver() override;

    [[nodiscard]] const QDir& directory() const;

    void setBasePath(const QString& basePath);

    void saveChartSnapshot(Widgets::Chart::ChartType chart,
                           const QImage& image,
                           Widgets::Chart::ChartView* chartView);
    bool saveReport(const Report& report, const QString& templatePath);

private:
    bool ensureDirectory();
    bool createAutoDirectory();
    bool chooseDirectoryManually();
    void cleanupEmptyDirectory();

private:
    QDir m_dir;
    QString m_basePath;
    bool m_isDirectoryCreated = false;
    QHash<QString, quint16> m_chartCounter;

signals:
    bool question(const QString& title, const QString& text);
    void setDirectoryToSave(const QString& currentPath, QString& result);
};
}