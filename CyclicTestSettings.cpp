#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"

#include <QMessageBox>

CyclicTestSettings::CyclicTestSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);



    ui->timeEdit->setMinimumTime(QTime(0, 0, 5));
    ui->timeEdit->setMaximumTime(QTime(0, 4, 0));

    connect(ui->pushButton_start, &QPushButton::clicked,
            this, &CyclicTestSettings::onPushButtonStartClicked);
}

CyclicTestSettings::~CyclicTestSettings()
{
    delete ui;
}

void CyclicTestSettings::onPushButtonStartClicked()
{
    m_parameters.holdTimeMs = ui->timeEdit->time().msecsSinceStartOfDay();

    auto *itemVals = ui->listWidget_value->currentItem();
    if (!itemVals) {
        QMessageBox::warning(this, "Ошибка", "Выберите строку со значениями.");
        return;
    }
    m_parameters.values.clear();
    for (auto s : itemVals->text().split('-', Qt::SkipEmptyParts)) {
        bool ok;
        quint16 v = s.toUShort(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Ошибка", "Неверный формат значений.");
            return;
        }
        m_parameters.values.append(v);
    }

    auto *itemSteps = ui->listWidget_step->currentItem();
    if (!itemSteps) {
        QMessageBox::warning(this, "Ошибка", "Выберите строку с задержками.");
        return;
    }
    m_parameters.delaysMs.clear();
    for (auto s : itemSteps->text().split('-', Qt::SkipEmptyParts)) {
        bool ok;
        quint32 sec = s.toUInt(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Ошибка", "Неверный формат задержек.");
            return;
        }
        m_parameters.delaysMs.append(sec * 1000u);
    }

    bool ok;
    quint32 cycles = ui->lineEdit_number_cycles->text().toUInt(&ok);
    if (!ok || cycles == 0) {
        QMessageBox::warning(this, "Ошибка", "Введите корректное число циклов (>0).");
        return;
    }
    m_parameters.numCycles = cycles;

    accept();
}

void CyclicTestSettings::onPushButtonCancelClicked()
{
    reject();
}
