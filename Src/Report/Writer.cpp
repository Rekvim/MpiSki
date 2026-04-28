#include "Writer.h"


namespace Report {
Writer::Writer(Saver::Report& report) : m_report(report) {}

void Writer::cell(const QString& sheet, quint16 row, quint16 col, const QVariant& value)
{
    m_report.data.push_back({sheet, row, col, value.toString()});
}

void Writer::image(const QString& sheet, quint16 row, quint16 col, const QImage& img)
{
    m_report.images.push_back({sheet, row, col, img});
}

void Writer::validation(const QString& formula, const QString& range)
{
    m_report.validation.push_back({formula, range});
}

}