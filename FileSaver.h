#ifndef FILESAVER_H
#define FILESAVER_H

#include <QDir>
#include <QObject>
#include "MyChart.h"
#include "Registry.h"

class FileSaver : public QObject
{
    Q_OBJECT
public:
    struct ExcelData
    {
        quint8 x;
        quint8 y;
        QLineEdit *line_edit;
    };

    struct ValidationData
    {
        QString formula;
        QString range;
    };

    struct Report
    {
        QVector<ExcelData> data;
        QVector<ValidationData> validation;
        QImage image1;
        QImage image2;
        QImage image3;
    };

    explicit FileSaver(QObject *parent = nullptr);
    void SetRegistry(Registry *registry);
    void SaveImage(MyChart *chart);
    QDir Directory();
    bool SaveReport(const Report &report);

private:
    void CreateDir();
    QDir dir_;
    bool created_;
    Registry *registry_;
signals:
    void Question(QString title, QString text, bool &result);
    void GetDirectory(QString current_path, QString &result);
};

#endif // FILESAVER_H
