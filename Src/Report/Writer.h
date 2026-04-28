#pragma once

#include <QtGlobal>

#include "Saver.h"

namespace Report {
    class Writer {
    public:
        explicit Writer(Saver::Report& report);

        void cell(const QString& sheet, quint16 row, quint16 col, const QVariant& value);
        void image(const QString& sheet, quint16 row, quint16 col, const QImage& img);
        void validation(const QString& formula, const QString& range);

    private:
        Saver::Report& m_report;
    };
}