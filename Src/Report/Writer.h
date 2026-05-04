#pragma once

#include <QString>
#include <QVariant>

#include "Data.h"

namespace Report {
    class Writer {
    public:
        explicit Writer(ReportData& report);

        void cell(const QString& sheet, quint16 row, quint16 col, const QVariant& value);
        void image(const QString& sheet, quint16 row, quint16 col, const QImage& img);
        void validation(const QString& formula, const QString& range);

    private:
        ReportData& m_report;
    };
}