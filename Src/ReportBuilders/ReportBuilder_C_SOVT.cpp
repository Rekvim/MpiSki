#include "ReportBuilder_C_SOVT.h"

ReportBuilder_C_SOVT::ReportBuilder_C_SOVT() {}

void ReportBuilder_C_SOVT::buildReport(
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
{
    cell(report, m_sheetCyclicTests, 1, 9, valveInfo.positionNumber);

    // Лист 1; Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, m_sheetCyclicTests, 4, 4, objectInfo.object);
    cell(report, m_sheetCyclicTests, 5, 4, objectInfo.manufactory);
    cell(report, m_sheetCyclicTests, 6, 4, objectInfo.department);

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, m_sheetCyclicTests, 4, 13, valveInfo.positionNumber);
    cell(report, m_sheetCyclicTests, 5, 13, valveInfo.serialNumber);
    cell(report, m_sheetCyclicTests, 6, 13, valveInfo.valveModel);
    cell(report, m_sheetCyclicTests, 7, 13, valveInfo.manufacturer);
    cell(report, m_sheetCyclicTests, 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, m_sheetCyclicTests, 9, 13, valveInfo.solenoidValveModel);
    cell(report, m_sheetCyclicTests, 10, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, m_sheetCyclicTests, 11, 13, QString::asprintf("%.2f", telemetryStore.supplyRecord.pressure_bar));
    cell(report, m_sheetCyclicTests, 12, 13, otherParams.safePosition);
    cell(report, m_sheetCyclicTests, 13, 13, valveInfo.driveModel);
    cell(report, m_sheetCyclicTests, 14, 13, otherParams.strokeMovement);
    cell(report, m_sheetCyclicTests, 15, 13, valveInfo.materialStuffingBoxSeal);

    // Страница:Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    cell(report, m_sheetCyclicTests, 20, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, m_sheetCyclicTests, 22, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, m_sheetCyclicTests, 24, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));
    cell(report, m_sheetCyclicTests, 26, 8, telemetryStore.cyclicTestRecord.sequenceShutoff);
    cell(report, m_sheetCyclicTests, 28, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecShutoff).toString("mm:ss.zzz"));

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    cell(report,
        m_sheetCyclicTests, 36, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        m_sheetCyclicTests, 38, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );

    const auto &ons  = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto &offs = telemetryStore.cyclicTestRecord.doOffCounts;
    for (int i = 0; i < ons.size(); ++i) {
        if (ons[i] == 0 && offs.value(i, 0) == 0)
            continue;

        quint16 row = 36 + quint16(i) * 2;
        cell(report,
            m_sheetCyclicTests, row, 10,
            QString::number(ons[i])
        );
        cell(report,
            m_sheetCyclicTests, row, 13,
            QString::number(offs.value(i, 0))
        );
    }

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    cell(report,
        m_sheetCyclicTests, 44, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        m_sheetCyclicTests, 44, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    );
    cell(report,
        m_sheetCyclicTests, 44, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    );

    cell(report,
        m_sheetCyclicTests, 46, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        m_sheetCyclicTests, 46, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    );
    cell(report,
        m_sheetCyclicTests, 46, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    );

    // Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, m_sheetCyclicTests, 53, 4, objectInfo.FIO);

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, m_sheetCyclicTests, 57, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
