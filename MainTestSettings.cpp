#include "MainTestSettings.h"
#include "ui_MainTestSettings.h"

MainTestSettings::MainTestSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainTestSettings)
{
    ui->setupUi(this);
}

MainTestSettings::~MainTestSettings()
{
    delete ui;
}

MainTestSettings::TestParameters MainTestSettings::getParameters() const
{
    TestParameters parameters;

    if (ui->tabWidget->currentIndex() == 0) {
        parameters.continuous = true;
        parameters.delay = 50;
        parameters.response = 50;
        parameters.pointNumbers = qreal(ui->timeEdit->time().msecsSinceStartOfDay()) / 50;
        parameters.signal_min = 3.0;
        parameters.signal_max = 21.0;
        parameters.is_cyclic = ui->checkBox_cyclic_test->isChecked();
        parameters.num_cycles = ui->spinBox_num_cycles->value();
        parameters.cycle_time = ui->timeEdit_cycle_time->time().msecsSinceStartOfDay() / 1000;
    } else {
        parameters.continuous = false;
        parameters.delay = ui->doubleSpinBox_delay->value() * 1000;
        parameters.pointNumbers = ui->doubleSpinBox_points->value();
        parameters.signal_min = ui->doubleSpinBox_signal_min->value();
        parameters.signal_max = ui->doubleSpinBox_signal_max->value();
        parameters.response = ui->doubleSpinBox_response->value() * 1000;
    }
    return parameters;
}
