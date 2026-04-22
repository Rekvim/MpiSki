#pragma once

#include <QObject>
#include <QDir>
#include <QImage>

#include "Src/Widgets/Chart/ChartView.h"
#include "Src/Storage/Registry.h"

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

        explicit Saver(QObject *parent = nullptr);

        void setRegistry(Registry *registry);
        void saveImage(Widgets::Chart::ChartView *chart);

        [[nodiscard]] const QDir &directory() const;
        void createDir();
        bool saveReport(const Report &report, const QString &templatePath);

    private:
        QDir m_dir;
        bool m_isDirectoryCreated = false;
        Registry *m_registry = nullptr;

    signals:
        bool question(const QString &title, const QString &text);
        void setDirectoryToSave(const QString &currentPath, QString &result);
    };
}