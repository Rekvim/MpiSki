#include "MainTestSettings.h"
#include "ui_MainTestSettings.h"

MainTestSettings::MainTestSettings(QWidget *parent) :
    AbstractTestSettings(parent),
    ui(new Ui::MainTestSettings)
{
    ui->setupUi(this);
}

MainTestSettings::~MainTestSettings()
{
    delete ui;
}

void MainTestSettings::readUi()
{
    if (ui->tabWidget->currentIndex() == 0) {
        m_params.continuous = true;
        m_params.delay = 50;
        m_params.response = 50;
        m_params.pointNumbers = qreal(ui->timeEdit->time().msecsSinceStartOfDay()) / 50;
        m_params.signal_min = 3.0;
        m_params.signal_max = 21.0;
    } /*else {
        testParameters.continuous = false;
        testParameters.delay = ui->doubleSpinBox_delay->value() * 1000;
        testParameters.pointNumbers = ui->doubleSpinBox_points->value();
        testParameters.signal_min = ui->doubleSpinBox_signal_min->value();
        testParameters.signal_max = ui->doubleSpinBox_signal_max->value();
        testParameters.response = ui->doubleSpinBox_response->value() * 1000;
    }*/
}

void MainTestSettings::accept()
{
    readUi();
    QDialog::accept();
}

void MainTestSettings::applyValveInfo(const ValveInfo& info)
{
    Q_UNUSED(info);
}

void MainTestSettings::applyPattern(SelectTests::PatternType pattern)
{
    Q_UNUSED(pattern);
}
