#include <QInputDialog>
#include "StepTestSettings.h"
#include "ui_StepTestSettings.h"

StepTestSettings::StepTestSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StepTestSettings)
{
    ui->setupUi(this);

    connect(ui->listWidget_value, &QListWidget::currentRowChanged, this, [=](int v) {
        ui->pushButton_change_value->setEnabled(v >= 0);
        ui->pushButton_delete_value->setEnabled((v >= 0) & (ui->listWidget_value->count() > 1));
    });

    connect(ui->pushButton_add_value, &QPushButton::clicked, this, [=]() {
        ui->listWidget_value->addItem("50.0");
        ui->pushButton_delete_value->setEnabled(ui->listWidget_value->currentRow() >= 0);
    });

    connect(ui->pushButton_delete_value, &QPushButton::clicked, this, [=]() {
        delete ui->listWidget_value->currentItem();

        if (ui->listWidget_value->count() == 1) {
            ui->pushButton_delete_value->setEnabled(false);
        }

        ui->listWidget_value->setCurrentRow(-1);
    });

    connect(ui->pushButton_change_value, &QPushButton::clicked, this, [=]() {
        bool ok;
        double d = QInputDialog::getDouble(this,
                                           "Ввод числа",
                                           "Значение:",
                                           ui->listWidget_value->currentItem()->text().toDouble(),
                                           0.0,
                                           100.0,
                                           1,
                                           &ok);

        if (ok) {
            ui->listWidget_value->currentItem()->setText(QString::asprintf("%.1f", d));
        }
    });

    connect(ui->timeEdit, &QTimeEdit::timeChanged, this, [&](QTime time) {
        if (time > max_time) {
            ui->timeEdit->setTime(max_time);
        }
        if (time < min_time) {
            ui->timeEdit->setTime(min_time);
        }
    });

    connect(ui->spinBox_T_value, &QSpinBox::valueChanged, this, [&](int value) {
        if (value < min_T_value) {
            ui->spinBox_T_value->setValue(min_T_value);
        }
        if (value > max_T_value) {
            ui->spinBox_T_value->setValue(max_T_value);
        }
    });

    ui->spinBox_T_value->setPrefix("T");
}

StepTestSettings::~StepTestSettings()
{
    delete ui;
}

void StepTestSettings::reverse()
{
    ui->listWidget_value->clear();
    ui->listWidget_value->addItem("90.0");
    ui->listWidget_value->addItem("80.0");
    ui->listWidget_value->addItem("70.0");
    ui->listWidget_value->addItem("60.0");
    ui->listWidget_value->addItem("50.0");
    ui->listWidget_value->addItem("40.0");
    ui->listWidget_value->addItem("30.0");
    ui->listWidget_value->addItem("20.0");
    ui->listWidget_value->addItem("10.0");
}

StepTestSettings::TestParameters StepTestSettings::getParameters()
{
    TestParameters test_parameters;

    test_parameters.delay = ui->timeEdit->time().msecsSinceStartOfDay();
    test_parameters.test_value = ui->spinBox_T_value->value();
    for (int i = 0; i < ui->listWidget_value->count(); i++) {
        test_parameters.points.append(ui->listWidget_value->item(i)->text().toDouble());
    }

    return test_parameters;
}
