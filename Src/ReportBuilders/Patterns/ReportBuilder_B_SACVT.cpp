#include "ReportBuilder_B_SACVT.h"
#include "Src/ReportBuilders/ReportWriter.h"
#include "Src/ReportBuilders/ReportBlocks/ObjectInfoBlock.h"
#include "Src/ReportBuilders/ReportBlocks/ValveSpecBlock.h"
#include "Src/ReportBuilders/ReportBlocks/SolenoidDetailsBlock.h"
#include "Src/ReportBuilders/ReportBlocks/CyclicSummaryBlock.h"

ReportBuilder_B_SACVT::ReportBuilder_B_SACVT() {}

void ReportBuilder_B_SACVT::buildReport(
    ReportSaver::Report& report,
    const TelemetryStore& telemetryStore,
    const ObjectInfo& objectInfo,
    const ValveInfo& valveInfo,
    const OtherParameters& otherParams,
    const QImage& imageChartTask,
    const QImage& imageChartPressure,
    const QImage& imageChartFriction,
    const QImage& imageChartStep
    )
{ // 294
    ReportWriter writer(report);

    ReportContext ctx{
        telemetryStore,
        objectInfo,
        valveInfo,
        otherParams,
        imageChartTask,
        imageChartPressure,
        imageChartFriction,
        imageChartStep
    };
    // Страница: 1;
    writer.cell(m_sheetCyclicTests, 1, 9, ctx.valve.positionNumber);

    ObjectInfoBlock({m_sheetCyclicTests, 4, 4 }).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 4, 13, true, true}).build(writer, ctx);
    CyclicSummaryBlock({m_sheetCyclicTests,
                           21,
                           8,
                           2
                       }, CyclicMode::Regulatory).build(writer, ctx);

    // Страница:Отчет ЦТ; Блок: Циклические испытания позиционера
    {
        const auto& ranges = telemetryStore.cyclicTestRecord.ranges;

        struct Agg {
            qreal rangePercent;
            qreal maxFwdVal = std::numeric_limits<qreal>::lowest();
            int maxFwdCycle = -1;
            qreal minRevVal = std::numeric_limits<qreal>::max();
            int minRevCycle = -1;
        };
        QMap<qreal, Agg> aggMap;

        // 1) Собираем все rec по ключу rangePercent
        for (const auto& rec : ranges) {
            auto it = aggMap.find(rec.rangePercent);
            if (it == aggMap.end()) {
                // первый раз для этого percent
                Agg a;
                a.rangePercent = rec.rangePercent;
                // прямой ход
                if (rec.maxForwardCycle >= 0) {
                    a.maxFwdVal   = rec.maxForwardValue;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход
                if (rec.maxReverseCycle >= 0) {
                    a.minRevVal   = rec.maxReverseValue;
                    a.minRevCycle = rec.maxReverseCycle;
                }
                aggMap.insert(rec.rangePercent, a);

            } else {
                // уже есть — обновляем экстремумы
                Agg &a = it.value();
                // прямой ход — берём глобальный максимум
                if (rec.maxForwardCycle >= 0
                    && rec.maxForwardValue > a.maxFwdVal) {
                    a.maxFwdVal   = rec.maxForwardValue;
                    a.maxFwdCycle = rec.maxForwardCycle;
                }
                // обратный ход — берём глобальный минимум
                if (rec.maxReverseCycle >= 0
                    && rec.maxReverseValue < a.minRevVal) {
                    a.minRevVal   = rec.maxReverseValue;
                    a.minRevCycle = rec.maxReverseCycle;
                }
            }
        }

        // 2) Выводим первые 10 (или сколько есть) диапазонов
        QVector<qreal> percents;
        percents.reserve(aggMap.size());
        for (auto it = aggMap.constBegin(); it != aggMap.constEnd(); ++it) {
            percents.append(it.key());
        }

        constexpr quint16 rowStart = 35, rowStep = 2;
        for (int i = 0; i < percents.size() && i < 10; ++i) {
            quint16 row = rowStart + i * rowStep;
            const Agg &a = aggMap[percents[i]];

            // Процент
            writer.cell(
                m_sheetCyclicTests, row, 2,
                QString::number(a.rangePercent)
            );

            // Прямой ход (максимум)
            if (a.maxFwdCycle >= 0) {
                writer.cell(
                    m_sheetCyclicTests, row, 8,
                    QString("%1")
                        .arg(a.maxFwdVal,   0, 'f', 2)
                );
                writer.cell(
                    m_sheetCyclicTests, row, 11,
                    QString("%1")
                        .arg(a.maxFwdCycle + 1)
                );
            } else {
                // нет данных
                writer.cell( m_sheetCyclicTests, row, 8, QString());
                writer.cell( m_sheetCyclicTests, row, 11, QString());
            }

            // Обратный ход (минимум)
            if (a.minRevCycle >= 0) {
                writer.cell(
                    m_sheetCyclicTests, row, 12,
                    QString("%1")
                        .arg(a.minRevVal,   0, 'f', 2)
                );
                writer.cell(
                    m_sheetCyclicTests, row, 15,
                    QString("%1")
                        .arg(a.minRevCycle + 1)
                );
            } else {
                writer.cell(  m_sheetCyclicTests, row, 12, QString());
                writer.cell(  m_sheetCyclicTests, row, 15, QString());
            }
        }
    }

    writer.cell(m_sheetCyclicTests, 58, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 62, 12, ctx.params.date);

    // Страница: 2;
    ObjectInfoBlock({m_sheetCyclicTests, 68, 4 }).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 68, 13, true, true}).build(writer, ctx);
    CyclicSummaryBlock({m_sheetCyclicTests,
                           85,
                           8,
                           2
                       }, CyclicMode::Shutoff).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 122, 4, objectInfo.FIO);
    writer.cell(m_sheetCyclicTests, 126, 12, otherParams.date);

    //Страница: 3;
    ObjectInfoBlock({m_sheetCyclicTests, 131, 4 }).build(writer, ctx);
    ValveSpecBlock({m_sheetCyclicTests, 131, 13, true, true}).build(writer, ctx);
    CyclicSummaryBlock({m_sheetCyclicTests,
                           148,
                           8,
                           2
                       }, CyclicMode::Shutoff).build(writer, ctx);

    SolenoidDetailsBlock({m_sheetCyclicTests,
                             164, // rowBase
                             8, // colCount
                             10, // colOn
                             13, // colOff
                             2, // rowStep
                             172, // rowSwitch1
                             174 // rowSwitch2
                         }).build(writer, ctx);

    writer.cell(m_sheetCyclicTests, 181, 4, ctx.object.FIO);
    writer.cell(m_sheetCyclicTests, 185, 12, ctx.params.date);
}
