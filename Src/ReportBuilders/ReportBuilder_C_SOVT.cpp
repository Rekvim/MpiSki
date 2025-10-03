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

    // Лист 1; Страница: Отчет ЦТ; Блок: Данные по объекту
    report.data.push_back({"Отчет ЦТ", 4, 4, objectInfo.object});
    report.data.push_back({"Отчет ЦТ", 5, 4, objectInfo.manufactory});
    report.data.push_back({"Отчет ЦТ", 6, 4, objectInfo.department});

    // Страница:Отчет ЦТ; Блок: Краткая спецификация на клапан
    report.data.push_back({"Отчет ЦТ", 4, 13, valveInfo.positionNumber});
    report.data.push_back({"Отчет ЦТ", 5, 13, valveInfo.serialNumber});
    report.data.push_back({"Отчет ЦТ", 6, 13, valveInfo.valveModel});
    report.data.push_back({"Отчет ЦТ", 7, 13, valveInfo.manufacturer});
    report.data.push_back({"Отчет ЦТ", 8, 13, QString("%1 / %2")
                                                  .arg(valveInfo.DN)
                                                  .arg(valveInfo.PN)});
    report.data.push_back({"Отчет ЦТ", 9, 13, valveInfo.solenoidValveModel});
    report.data.push_back({"Отчет ЦТ", 10, 13, QString("%1 / %2")
                                                   .arg(valveInfo.limitSwitchModel)
                                                   .arg(valveInfo.materialStuffingBoxSeal)});
    report.data.push_back({"Отчет ЦТ",11, 13, QString::asprintf("%.2f bar", telemetryStore.supplyRecord.pressure_bar)});
    report.data.push_back({"Отчет ЦТ", 12, 13, otherParams.safePosition});
    report.data.push_back({"Отчет ЦТ", 13, 13, valveInfo.driveModel});
    report.data.push_back({"Отчет ЦТ", 14, 13, otherParams.strokeMovement});
    report.data.push_back({"Отчет ЦТ", 15, 13, valveInfo.materialStuffingBoxSeal});

    // Страница:Отчет ЦТ; Блок: РЕЗУЛЬТАТЫ ИСПЫТАНИЙ СОЛЕНОИДА/КОНЦЕВОГО ВЫКЛЮЧАТЕЛЯ
    report.data.push_back({"Отчет ЦТ", 20, 8, telemetryStore.strokeTestRecord.timeForwardMs});
    report.data.push_back({"Отчет ЦТ", 22, 8, telemetryStore.strokeTestRecord.timeBackwardMs});
    report.data.push_back({"Отчет ЦТ", 24, 8, QString::number(telemetryStore.cyclicTestRecord.cycles) });
    report.data.push_back({"Отчет ЦТ", 26, 8, telemetryStore.cyclicTestRecord.sequence });
    report.data.push_back({"Отчет ЦТ", 29, 8, QTime(0,0).addSecs(telemetryStore.cyclicTestRecord.totalTimeSec).toString("mm:ss.zzz")});

    // Страница:Отчет ЦТ; Блок: Циклические испытания соленоидного клапана
    report.data.push_back({
        "Отчет ЦТ", 36, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 38, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });

    const auto &ons  = telemetryStore.cyclicTestRecord.doOnCounts;
    const auto &offs = telemetryStore.cyclicTestRecord.doOffCounts;
    for (int i = 0; i < ons.size(); ++i) {
        if (ons[i] == 0 && offs.value(i, 0) == 0)
            continue;

        quint16 row = 36 + quint16(i) * 2;
        report.data.push_back({
            "Отчет ЦТ", row, 10,
            QString::number(ons[i])
        });
        report.data.push_back({
            "Отчет ЦТ", row, 13,
            QString::number(offs.value(i, 0))
        });
    }

    // Страница:Отчет ЦТ; Блок: Циклические испытания концевого выключателя/датчика положения
    report.data.push_back({
        "Отчет ЦТ", 44, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 44, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 44, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    });

    report.data.push_back({
        "Отчет ЦТ", 46, 8,
        QString::number(telemetryStore.cyclicTestRecord.cycles)
    });
    report.data.push_back({
        "Отчет ЦТ", 46, 10,
        QString::number(telemetryStore.cyclicTestRecord.switch0to3Count)
    });
    report.data.push_back({
        "Отчет ЦТ", 46, 13,
        QString::number(telemetryStore.cyclicTestRecord.switch3to0Count)
    });

    // Страница: Отчет ЦТ; Блок: Исполнитель
    report.data.push_back({"Отчет ЦТ", 53, 4, objectInfo.FIO});

    // Страница: Отчет ЦТ; Блок: Дата
    report.data.push_back({"Отчет ЦТ", 57, 12, otherParams.date});

    report.validation.push_back({"=ЗИП!$A$1:$A$37", "J56:J65"});
    report.validation.push_back({"=Заключение!$B$1:$B$4", "E42"});
    report.validation.push_back({"=Заключение!$C$1:$C$3", "E44"});
    report.validation.push_back({"=Заключение!$E$1:$E$4", "E46"});
    report.validation.push_back({"=Заключение!$D$1:$D$5", "E48"});
    report.validation.push_back({"=Заключение!$F$3", "E50"});
}
