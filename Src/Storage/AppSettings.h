#pragma once

#include <QSettings>

class AppSettings
{
public:
    AppSettings();

    std::pair<double,double> linearRange() const;
    void setLinearRange(double min,double max);

private:
    void setValue(const QString& key, const QVariant& val);

    QSettings m_settings;
};