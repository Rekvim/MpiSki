#include "TelemetryUiMapper.h"
#include "ui_MainWindow.h"

void TelemetryUiMapper::updateInit(const InitState& init)
{
    m_ui->label_deviceStatusValue->setText(init.deviceStatusText);
    m_ui->label_deviceStatusValue->setStyleSheet(
        "color:" + init.deviceStatusColor.name(QColor::HexRgb));

    m_ui->label_deviceInitValue->setText(init.initStatusText);
    m_ui->label_deviceInitValue->setStyleSheet(
        "color:" + init.initStatusColor.name(QColor::HexRgb));

    m_ui->label_connectedSensorsNumber->setText(init.connectedSensorsText);
    m_ui->label_connectedSensorsNumber->setStyleSheet(
        "color:" + init.connectedSensorsColor.name(QColor::HexRgb));

    m_ui->label_startingPositionValue->setText(init.startingPositionText);
    m_ui->label_startingPositionValue->setStyleSheet(
        "color:" + init.startingPositionColor.name(QColor::HexRgb));

    m_ui->label_finalPositionValue->setText(init.finalPositionText);
    m_ui->label_finalPositionValue->setStyleSheet(
        "color:" + init.finalPositionColor.name(QColor::HexRgb));
}

void TelemetryUiMapper::updateMainTest(const Telemetry& t)
{
    if (!t.testMain) return;

    m_ui->label_pressureDifferenceValue->setText(
        QString("%1")
            .arg(t.testMain->pressureDiff, 0, 'f', 3)
        );
    m_ui->label_frictionForceValue->setText(
        QString("%1")
            .arg(t.testMain->frictionForce, 0, 'f', 3)
        );
    m_ui->label_frictionPercentValue->setText(
        QString("%1")
            .arg(t.testMain->frictionPercent, 0, 'f', 2)
        );
    m_ui->lineEdit_resultsTable_frictionForceValue->setText(
        QString("%1")
            .arg(t.testMain->frictionForce, 0, 'f', 3)
        );
    m_ui->lineEdit_resultsTable_frictionPercentValue->setText(
        QString("%1")
            .arg(t.testMain->frictionPercent, 0, 'f', 2)
        );

    m_ui->label_dynamicErrorMeanPercent->setText(
        QString("%1 %")
            .arg(t.testMain->dynamicErrorMeanPercent, 0, 'f', 2)
        );
    m_ui->label_dynamicErrorMean->setText(
        QString("%1 мА")
            .arg(t.testMain->dynamicErrorMean, 0, 'f', 3)
        );
    m_ui->label_dynamicErrorMaxPercent->setText(
        QString("%1 %")
            .arg(t.testMain->dynamicErrorMaxPercent, 0, 'f', 2)
        );

    m_ui->label_dynamicErrorMax->setText(
        QString("%1 мА")
            .arg(t.testMain->dynamicErrorMax, 0, 'f', 3)
        );
    m_ui->lineEdit_resultsTable_dynamicErrorReal->setText(
        QString("%1")
            .arg(t.testMain->dynamicErrorReal, 0, 'f', 2)
        );

    m_ui->label_dynamicErrorMax->setText(
        QString("%1 бар")
            .arg(t.testMain->lowLimitPressure, 0, 'f', 2)
        );
    m_ui->label_dynamicErrorMax->setText(
        QString("%1 бар")
            .arg(t.testMain->highLimitPressure, 0, 'f', 2)
        );

    m_ui->lineEdit_resultsTable_strokeReal->setText(
        QString("%1").arg(t.valveStrokeRecord.real, 0, 'f', 2));

    m_ui->label_lowLimitValue->setText(
        QString("%1")
            .arg(t.testMain->lowLimitPressure)
        );
    m_ui->label_highLimitValue->setText(
        QString("%1")
            .arg(t.testMain->highLimitPressure)
        );

    m_ui->lineEdit_resultsTable_rangePressure->setText(
        QString("%1–%2")
            .arg(t.testMain->lowLimitPressure, 0, 'f', 2)
            .arg(t.testMain->highLimitPressure, 0, 'f', 2)
        );

    m_ui->lineEdit_resultsTable_driveRangeReal->setText(
        QString("%1–%2")
            .arg(t.testMain->springLow, 0, 'f', 2)
            .arg(t.testMain->springHigh, 0, 'f', 2)
        );

    m_ui->label_valveStroke_range->setText(
        QString("%1")
            .arg(t.valveStrokeRecord.range)
        );


}

void TelemetryUiMapper::updateStrokeTest(const Domain::Tests::Stroke::Result& r)
{
    m_ui->lineEdit_strokeTest_forwardTime->setText(r.timeForwardMs);
    m_ui->lineEdit_resultsTable_strokeTest_forwardTime->setText(r.timeForwardMs);

    m_ui->lineEdit_strokeTest_backwardTime->setText(r.timeBackwardMs);
    m_ui->lineEdit_resultsTable_strokeTest_backwardTime->setText(r.timeBackwardMs);
}

void TelemetryUiMapper::updateCrossing(const Telemetry& t)
{
    m_ui->lineEdit_crossingLimits_dynamicError_value->setText(
        QString::number(t.testMain->dynamicErrorReal, 'f', 2));

    m_ui->lineEdit_crossingLimits_linearCharacteristic_value->setText(
        QString::number(t.testMain->linearityError, 'f', 2));

    m_ui->lineEdit_crossingLimits_range_value->setText(
        QString::number(t.valveStrokeRecord.real, 'f', 2));

    m_ui->lineEdit_crossingLimits_spring_value->setText(
        QString("%1–%2")
            .arg(t.testMain->springLow, 0, 'f', 2)
            .arg(t.testMain->springHigh, 0, 'f', 2));

    m_ui->lineEdit_crossingLimits_coefficientFriction_value->setText(
        QString::number(t.testMain->frictionPercent, 'f', 2));
}
