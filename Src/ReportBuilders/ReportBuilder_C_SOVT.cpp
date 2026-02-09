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
    cell(report, "Отчет ЦТ", 1, 9, valveInfo.positionNumber);

    // Лист 1; Страница: Отчет ЦТ; Блок: Данные по объекту
    cell(report, "Отчет ЦТ", 4, 4, objectInfo.object);
    cell(report, "Отчет ЦТ", 5, 4, objectInfo.manufactory);
    cell(report, "Отчет ЦТ", 6, 4, objectInfo.department);

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    cell(report, "Отчет ЦТ", 4, 13, valveInfo.positionNumber);
    cell(report, "Отчет ЦТ", 5, 13, valveInfo.serialNumber);
    cell(report, "Отчет ЦТ", 6, 13, valveInfo.valveModel);
    cell(report, "Отчет ЦТ", 7, 13, valveInfo.manufacturer);
    cell(report, "Отчет ЦТ", 8, 13, QString("%1 / %2").arg(valveInfo.DN, valveInfo.PN));
    cell(report, "Отчет ЦТ", 9, 13, valveInfo.solenoidValveModel);
    cell(report, "Отчет ЦТ", 10, 13, QString("%1 / %2").arg(valveInfo.limitSwitchModel, valveInfo.positionSensorModel));
    cell(report, "Отчет ЦТ",11, 13, QString::asprintf("%.2f", telemetryStore.supplyRecord.pressure_bar));
    cell(report, "Отчет ЦТ", 12, 13, otherParams.safePosition);
    cell(report, "Отчет ЦТ", 13, 13, valveInfo.driveModel);
    cell(report, "Отчет ЦТ", 14, 13, otherParams.strokeMovement);
    cell(report, "Отчет ЦТ", 15, 13, valveInfo.materialStuffingBoxSeal);

    // Страница:Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    cell(report, "Отчет ЦТ", 20, 8, telemetryStore.strokeTestRecord.timeForwardMs);
    cell(report, "Отчет ЦТ", 22, 8, telemetryStore.strokeTestRecord.timeBackwardMs);
    cell(report, "Отчет ЦТ", 24, 8, QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff));
    cell(report, "Отчет ЦТ", 26, 8, telemetryStore.cyclicTestRecord.sequenceShutoff);
    cell(report, "Отчет ЦТ", 28, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSecShutoff).toString("mm:ss.zzz"));

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    cell(report,
        "Отчет ЦТ", 36, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        "Отчет ЦТ", 38, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );

    const auto &ons  = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto &offs = telemetryStore.cyclicTestRecord.doOffCounts;
    for (int i = 0; i < ons.size(); ++i) {
        if (ons[i] == 0 && offs.value(i, 0) == 0)
            continue;

        quint16 row = 36 + quint16(i) * 2;
        cell(report,
            "Отчет ЦТ", row, 10,
            QString::number(ons[i])
        );
        cell(report,
            "Отчет ЦТ", row, 13,
            QString::number(offs.value(i, 0))
        );
    }

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    cell(report,
        "Отчет ЦТ", 44, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        "Отчет ЦТ", 44, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    );
    cell(report,
        "Отчет ЦТ", 44, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    );

    cell(report,
        "Отчет ЦТ", 46, 8,
        QString::number(telemetryStore.cyclicTestRecord.numCyclesShutoff)
    );
    cell(report,
        "Отчет ЦТ", 46, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    );
    cell(report,
        "Отчет ЦТ", 46, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    );

    // Страница: Отчет ЦТ; Блок: Исполнитель
    cell(report, "Отчет ЦТ", 53, 4, objectInfo.FIO);

    // Страница: Отчет ЦТ; Блок: Дата
    cell(report, "Отчет ЦТ", 57, 12, otherParams.date);

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
