#pragma once

#include <QObject>
#include <QDir>
#include <QHash>
#include <QString>

#include "Data.h"
#include "Widgets/Chart/ChartType.h"

namespace Widgets::Chart {
class ChartView;
}

namespace Report {

class Saver : public QObject
{
    Q_OBJECT

public:
    explicit Saver(QObject* parent = nullptr);
    ~Saver() override;

    [[nodiscard]] const QDir& directory() const;

    void setBasePath(const QString& basePath);

    void saveChartSnapshot(
        Widgets::Chart::ChartType chart,
        const QImage& image,
        Widgets::Chart::ChartView* chartView);

    bool saveReport(const ReportData& report, const QString& templatePath);

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