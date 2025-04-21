#include <QApplication>
#include <QSettings>
#include "MpiSettings.h"

MPI_Settings::MPI_Settings(QObject *parent)
    : QObject{parent}
{
    QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    settings.beginGroup("ADC");
    for (int i = 0; i < 6; i++) {
        ADC_.push_back(settings.value("ADC" + QString::number(i), "20.625").toDouble());
    }
    settings.endGroup();

    settings.beginGroup("Sensors");
    settings.beginGroup("Linear");
    Sensors_.push_back(
        {settings.value("min", "0.0").toDouble(), settings.value("max", "50.0").toDouble()});
    settings.endGroup();

    for (int i = 1; i < 4; i++) {
        settings.beginGroup("Pressure" + QString::number(i));
        Sensors_.push_back(
            {settings.value("min", "-1.0").toDouble(), settings.value("max", "9.0").toDouble()});
        settings.endGroup();
    }
    settings.endGroup();

    settings.beginGroup("DAC");
    DAC_.bias = settings.value("Bias", "0.0").toDouble();
    DAC_.min = settings.value("Min", "3.0").toDouble();
    DAC_.max = settings.value("Max", "21.0").toDouble();

    if (DAC_.min < 3.0) {
        DAC_.min = 3.0;
    }

    if (DAC_.max > 21.0) {
        DAC_.max = 21.0;
    }

    if (DAC_.min >= DAC_.max) {
        DAC_.min = 3.0;
        DAC_.max = 21.0;
    }

    settings.endGroup();
}


qreal MPI_Settings::GetADC(quint8 num) const
{
    assert(num < ADC_.size());
    return ADC_.at(num);
}

MPI_Settings::MinMax MPI_Settings::GetSensor(quint8 num) const
{
    assert(num < Sensors_.size());
    return Sensors_.at(num);
}

MPI_Settings::DAC MPI_Settings::GetDAC() const
{
    return DAC_;
}
