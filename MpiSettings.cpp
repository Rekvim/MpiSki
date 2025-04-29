#include "MpiSettings.h"

MPI_Settings::MPI_Settings(QObject *parent)
    : QObject{parent}
{
    QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    settings.beginGroup("ADC");
    for (int i = 0; i < 6; i++) {
        m_ADC.push_back(settings.value("ADC" + QString::number(i), "20.625").toDouble());
    }
    settings.endGroup();

    settings.beginGroup("Sensors");
    settings.beginGroup("Linear");
    m_sensors.push_back(
        {settings.value("min", "0.0").toDouble(), settings.value("max", "50.0").toDouble()});
    settings.endGroup();

    for (int i = 1; i < 4; i++) {
        settings.beginGroup("Pressure" + QString::number(i));
        m_sensors.push_back(
            {settings.value("min", "-1.0").toDouble(), settings.value("max", "9.0").toDouble()});
        settings.endGroup();
    }
    settings.endGroup();

    settings.beginGroup("DAC");
    m_DAC.bias = settings.value("Bias", "0.0").toDouble();
    m_DAC.min = settings.value("Min", "3.0").toDouble();
    m_DAC.max = settings.value("Max", "21.0").toDouble();

    if (m_DAC.min < 3.0) {
        m_DAC.min = 3.0;
    }

    if (m_DAC.max > 21.0) {
        m_DAC.max = 21.0;
    }

    if (m_DAC.min >= m_DAC.max) {
        m_DAC.min = 3.0;
        m_DAC.max = 21.0;
    }

    settings.endGroup();
}


qreal MPI_Settings::GetADC(quint8 num) const
{
    assert(num < m_ADC.size());
    return m_ADC.at(num);
}

MPI_Settings::MinMax MPI_Settings::GetSensor(quint8 num) const
{
    assert(num < m_sensors.size());
    return m_sensors.at(num);
}

MPI_Settings::DAC MPI_Settings::GetDAC() const
{
    return m_DAC;
}
