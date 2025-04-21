#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"
#include <QStringList>
#include <QString>

CyclicTestSettings::CyclicTestSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &CyclicTestSettings::accept);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &CyclicTestSettings::reject);
}

CyclicTestSettings::~CyclicTestSettings()
{
    delete ui;
}

CyclicTestSettings::TestParameters CyclicTestSettings::getParameters()
{
    TestParameters params;

    params.delay = 10000;

    bool ok;
    params.num_cycles = ui->lineEdit_number_cycles->text().toUInt(&ok);
    if (!ok) {
        params.num_cycles = 1;
    }

    if (QListWidgetItem *item = ui->listWidget_value->currentItem()) {
        QStringList value_strs = item->text().split("-", Qt::SkipEmptyParts);
        for (const QString &val : value_strs) {
            params.points.append(val.toDouble());
        }
    }

    for (int i = 0; i < ui->listWidget_step->count(); ++i) {
        QString val = ui->listWidget_step->item(i)->text();
        params.steps.append(val.toDouble());
    }

    return params;
}
