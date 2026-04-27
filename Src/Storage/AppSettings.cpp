#include "AppSettings.h"

#include <QCoreApplication>

AppSettings::AppSettings()
    : m_settings(
          QCoreApplication::applicationDirPath() + "/settings.ini",
          QSettings::IniFormat) {}

void AppSettings::setValue(const QString& key, const QVariant& val) {
    m_settings.setValue(key,val);
}

std::pair<double, double> AppSettings::linearRange() const
{
    double min = m_settings.value("Sensors/Linear/min", 0).toDouble();
    double max = m_settings.value("Sensors/Linear/max", 50).toDouble();

    return {min, max};
}

void AppSettings::setLinearRange(double min, double max)
{
    setValue("Sensors/Linear/min", min);
    setValue("Sensors/Linear/max", max);
}
