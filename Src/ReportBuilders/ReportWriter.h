#ifndef REPORTWRITER_H
#define REPORTWRITER_H

#include "ReportSaver.h"

class ReportWriter {
public:
    explicit ReportWriter(ReportSaver::Report& report)
        : m_report(report) {}

    void cell(const QString& sheet,
              quint16 row,
              quint16 col,
              const QVariant& value)
    {
        m_report.data.push_back({sheet, row, col, value.toString()});
    }

    void image(const QString& sheet,
               quint16 row,
               quint16 col,
               const QImage& img)
    {
        m_report.images.push_back({sheet, row, col, img});
    }

    void validation(const QString& formula,
                    const QString& range)
    {
        m_report.validation.push_back({formula, range});
    }

private:
    ReportSaver::Report& m_report;
};

#endif // REPORTWRITER_H
