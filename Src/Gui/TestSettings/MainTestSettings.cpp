#include "MainTestSettings.h"
#include "ui_MainTestSettings.h"

MainTestSettings::MainTestSettings(QWidget *parent) :
    BaseSequenceSettingsDialog(parent),
    ui(new Ui::MainTestSettings)
{
    ui->setupUi(this);
}

MainTestSettings::~MainTestSettings()
{
    delete ui;
}

Domain::Tests::Main::Params
MainTestSettings::readParamsFromUi() const
{
    Domain::Tests::Main::Params p;

    p.continuous = true;
    p.delay = 50;
    p.response = 50;
    p.pointNumbers = qreal(ui->timeEdit->time().msecsSinceStartOfDay()) / 50;
    p.signal_min = 3.0;
    p.signal_max = 21.0;

    return p;
}

Domain::Tests::Main::Params
MainTestSettings::parameters() const
{
    return readParamsFromUi();
}

void MainTestSettings::applyValveInfo(const ValveInfo& info)
{
    Q_UNUSED(info);
}

void MainTestSettings::applyProfile(const Domain::DeviceProfile&)
{
}
