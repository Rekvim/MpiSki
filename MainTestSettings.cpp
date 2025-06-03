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
    TestParameters testParameters;

    if (ui->tabWidget->currentIndex() == 0) {
        testParameters.continuous = true;
        testParameters.delay = 50;
        testParameters.response = 50;
        testParameters.pointNumbers = qreal(ui->timeEdit->time().msecsSinceStartOfDay()) / 50;
        testParameters.signal_min = 3.0;
        testParameters.signal_max = 21.0;

    } else {
        testParameters.continuous = false;
        testParameters.delay = ui->doubleSpinBox_delay->value() * 1000;
        testParameters.pointNumbers = ui->doubleSpinBox_points->value();
        testParameters.signal_min = ui->doubleSpinBox_signal_min->value();
        testParameters.signal_max = ui->doubleSpinBox_signal_max->value();
        testParameters.response = ui->doubleSpinBox_response->value() * 1000;
    }
    return testParameters;
}

qint64 MainTestSettings::totalTestTimeMillis() const
{
    TestParameters testParameters = getParameters();

    if (testParameters.continuous) {
        return ui->timeEdit->time().msecsSinceStartOfDay();
    } else {
        qint64 forwardTime = qint64(testParameters.pointNumbers * testParameters.response);
        qint64 backwardTime = forwardTime;
        return testParameters.delay + forwardTime + testParameters.delay + backwardTime;
    }
}
