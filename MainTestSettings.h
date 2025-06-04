#ifndef MAINTESTSETTINGS_H
#define MAINTESTSETTINGS_H

#pragma once
#include <QDialog>

namespace Ui {
class MainTestSettings;
}

class MainTestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit MainTestSettings(QWidget *parent = nullptr);
    ~MainTestSettings();

    struct TestParameters
    {
        bool continuous;
        quint64 Time;
        QList<qreal> points;
        QList<qreal> steps;
        quint64 delay;
        quint16 pointNumbers;
        qreal signal_min;
        qreal signal_max;
        quint16 response;
        quint16 dac_min;
        quint16 dac_max;
        bool is_cyclic = false;
        quint16 num_cycles;
        quint16 cycle_time;
    };

    TestParameters getParameters() const;
    qint64 totalTestTimeMillis() const;

private:
    Ui::MainTestSettings *ui;
};

#endif // MAINTESTSETTINGS_H
